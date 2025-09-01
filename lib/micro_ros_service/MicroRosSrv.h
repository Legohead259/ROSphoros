#include <MicroRosController.h>
#include <rosphoros_interfaces/srv/set_gpio_state.h>

using SetGPIOState_Request = rosphoros_interfaces__srv__SetGPIOState_Request;
using SetGPIOState_Response = rosphoros_interfaces__srv__SetGPIOState_Response;

enum GPIO_STATE {
    OFF,
    ON,
    TOGGLE
};

enum RESPONSE_CODE {
    SUCCESS,
    FAILED_GENERAL,
    FAILED_NOT_VALID,
    FAILED_BLOCKED
};

class MicroRosSrv : MicroRosController {
public:
    MicroRosSrv();
    ~MicroRosSrv() { destroyEntities(); }

    bool begin();
    
protected:
    void handleConnectionState() override { MicroRosController::handleConnectionState(); }
    bool createEntities() override;
    void destroyEntities() override;

private:
    static MicroRosSrv* instance_;
    TaskHandle_t microRosTask;

    // ROS node configuration
    const char* publisherTopic="micro_ros_response";
    const char* subscriberTopic="micro_ros_name";

    // ROS entities
    rcl_service_t setGpioStateService;
    SetGPIOState_Request setGpioStateRequest;
    SetGPIOState_Response setGpioStateResponse;

    void _setGpioStateCallback(const void* req, void* res);

    static void _setGpioStateCallbackStatic(const void* req, void* res) { instance_->_setGpioStateCallback(req, res); }

    static void microRosTaskCallbackStatic(void* pvParameters) {
        for (;;) {
            instance_->handleConnectionState();
            vTaskDelay(pdMS_TO_TICKS(10));  // Yield to scheduler every 10 ms
        }
    }
};

extern MicroRosSrv controllerSrv;