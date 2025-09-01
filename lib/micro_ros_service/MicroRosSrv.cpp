#include "MicroRosSrv.h"

MicroRosSrv::MicroRosSrv() {}

bool MicroRosSrv::begin() {
    instance_ = this;
    
    xTaskCreate(
        microRosTaskCallbackStatic,     // Function to implement the task
        "MicroROS::main",               // Name of the task
        4096,                           // Stack size in words
        nullptr,                        // Task input parameter
        1,                              // Priority of the task
        &microRosTask                   // Task handle
    );

    return MicroRosController::begin();
}

bool MicroRosSrv::createEntities() {
    allocator = rcl_get_default_allocator();

    // Initialize options and set domain ID
    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    if (rcl_init_options_init(
        &init_options, 
        allocator) != RCL_RET_OK) {
        return false;
    }
    if (rcl_init_options_set_domain_id(
        &init_options, 
        domainID) != RCL_RET_OK) {
        return false;
    }

    // Initialize support with domain ID options
    if (rclc_support_init_with_options(
        &support, 
        0,
        NULL, 
        &init_options, 
        &allocator) != RCL_RET_OK) {
        return false;
    }

    // Clean up initialization options
    if (rcl_init_options_fini(&init_options) != RCL_RET_OK) {
        return false;
    }

    // Initialize node and rest of entities
    if (rclc_node_init_default(
        &node, 
        nodeName, 
        nodeNamespace, 
        &support) != RCL_RET_OK) {
        return false;
    }

    // Create publisher
    if (rclc_service_init_default(
        &setGpioStateService, 
        &node, 
        ROSIDL_GET_SRV_TYPE_SUPPORT(rosphoros_interfaces, srv, SetGPIOState), 
        "/set_gpio_state") != RCL_RET_OK) {
        return false;
    }

    // Initialize executor
    if (rclc_executor_init(
        &executor, 
        &support.context, 
        2, 
        &allocator) != RCL_RET_OK) {
        return false;
    }

    // Add service to executor
    if (rclc_executor_add_service(
        &executor, 
        &setGpioStateService,
        &setGpioStateRequest, 
        &setGpioStateResponse, 
        _setGpioStateCallbackStatic) != RCL_RET_OK) {
        return false;
    }

    return true;
}

void MicroRosSrv::destroyEntities() {
    rmw_context_t* rmw_context = rcl_context_get_rmw_context(&support.context);
    (void)rmw_uros_set_context_entity_destroy_session_timeout(rmw_context, 0);

    rcl_ret_t rc = RCL_RET_OK;
    rc = rcl_service_fini(&setGpioStateService, &node);
    rclc_executor_fini(&executor);
    rc = rcl_node_fini(&node);
    rclc_support_fini(&support);
}

void MicroRosSrv::_setGpioStateCallback(const void* req, void* res) {
    SetGPIOState_Request* req_in = (SetGPIOState_Request*) req;
    SetGPIOState_Response* res_in = (SetGPIOState_Response*) res;

    // TODO: Implement GPIO number validation
    // TODO: Implement more complex result validation
    switch (req_in->mode) {
    case GPIO_STATE::OFF:
        digitalWrite(req_in->gpio, LOW);
        res_in->result = RESPONSE_CODE::SUCCESS ? !digitalRead(req_in->gpio) : RESPONSE_CODE::FAILED_GENERAL;
        break;
    case GPIO_STATE::ON:
        digitalWrite(req_in->gpio, HIGH);
        res_in->result = RESPONSE_CODE::SUCCESS ? digitalRead(req_in->gpio) : RESPONSE_CODE::FAILED_GENERAL;
        break;
    case GPIO_STATE::TOGGLE:
        digitalWrite(req_in->gpio, !digitalRead(req_in->gpio));
        res_in->result = RESPONSE_CODE::SUCCESS ? (digitalRead(req_in->gpio) == !digitalRead(req_in->gpio)) : RESPONSE_CODE::FAILED_GENERAL;
        break;
    default:
        res_in->result = RESPONSE_CODE::FAILED_GENERAL;
        break;
    }
}

MicroRosSrv* MicroRosSrv::instance_ = nullptr;
MicroRosSrv controllerSrv;