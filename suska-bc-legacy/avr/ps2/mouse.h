#define MS_RESET                    0xFF  // CMD für Reset der Maus (->ACK)
#define MS_RESEND                   0xFE  // CMD für letztes Zeichen erneut senden (bei Fehler)
#define MS_ERROR                    0xFC  // -> Fehlermeldung anstelle eines ACK
#define MS_ACK                      0xFA  // -> Bestätigung der Maus
#define MS_SET_DEFAULTS             0xF6  // Standardeinstellung, Stream-Mode (->ACK)
#define MS_DISABLE_DATA_REPORTING   0xF5  // deaktivieren (->ACK)
#define MS_ENABLE_DATA_REPORTING    0xF4  // wieder aktivieren (->ACK)
#define MS_SET_SAMPLE_RATE          0xF3  // (->ACK), dann Rate senden (->ACK)
#define MS_GET_DEVICE_ID            0xF2  // (->ACK, Device-ID) =0x00 für PS2
#define MS_SET_REMOTE_MODE          0xF0  // remote-mode (->ACK)
#define MS_SET_WRAP_MODE            0xEE  // wrap-mode (->ACK)
#define MS_RESET_WRAP_MODE          0xEC  // end wrap-mode (->ACK)
#define MS_READ_DATA                0xEB  // (->ACK + 3 Byte movement data packet)
#define MS_SET_STREAM_MODE          0XEA  // stream-mode (->ACK)
#define MS_STATUS_REQUEST           0xE9  // (->ACK + 3 Byte status packet)
#define MS_SET_RESOLUTION           0xE8  // (->ACK) dann resolution senden (->ACK)
#define MS_SET_SCALING_2_1          0xE7  // (->ACK)
#define MS_SET_SCALING_1_1          0xE6  // (->ACK)

#define Y_OVF                       7     // Die Bedeutung der Bits in ms_ctrl
#define X_OVF                       6
#define Y_SIGN                      5
#define X_SIGN                      4
#define BUTTON_M                    2
#define BUTTON_R                    1
#define BUTTON_L                    0

void mouse_init(void);
void mouse_clear_buffer(void);
void mouse_poll(void);
uint8_t mouse_send_cmd(uint8_t cmd);
uint8_t mouse_get(void);
uint8_t is_mouse_available(void);


