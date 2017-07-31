
#pragma once

#include "fx.h"
#include "fx3d.h"

#include "engine_api/RtcCapture.h"

#define PRINT_YESNO(x) ( x ? "yes" : "no" )

#define UPLOADBITRATENUM 100

// Timer setting (in nanoseconds)
const FXTime TIMER_INTERVAL = 1000000000;

/*******************************************************************************/

// Event Handler Object
class UploadWindow : public FXMainWindow {
  FXDECLARE(UploadWindow)

public:
  enum{
    ID_CANVAS = FXMainWindow::ID_LAST,
    ID_SPIN,
    ID_SPINFAST,
    ID_STOP,
    ID_TIMEOUT,
    ID_CHORE,
    ID_OPENGL,
    ID_MULTISAMPLE_OFF,
    ID_MULTISAMPLE_2X,
    ID_MULTISAMPLE_4X,
    ID_CAMERA,
    ID_AUDIO,

    ID_CREATE_STREAM,
    ID_UPLOAD_INIT,
    ID_UPLOAD_START,
    ID_AUTO_UPLOAD,
    ID_LOST_RATE,
    ID_FRAME_RATE, //fps
    ID_ENABLE_FEC,
    ID_ENABLE_NACK,
    ID_ENABLE_AFPS,//adapt fps
    ID_ENABLE_NETCHANGE,
    ID_PLAYER,

    ID_OPEN_CAMERA,
    ID_CLOSE_CAMERA,
    ID_COMBO_CAMERA,
    ID_COMBO_RES,
    ID_COMBO_AUDIO,
    ID_COMBO_LAPI,
    ID_COMBO_REPORT,
    ID_OPEN_AUDIO,
    ID_CHECK_AUTO,
    ID_BITRATE_LIST,
    ID_FRAME_LIST,
    ID_FRAMEDETAIL_LIST,
    ID_COMBO_LOGLEVEL,
    ID_COMBO_PROTOCAL,

    ID_SPEED
  };

public:
  // UploadWindow constructor
  UploadWindow(FXApp* a);

  virtual void create();
  virtual void destroy();

  // UploadWindow destructor
  virtual ~UploadWindow();

protected:
  UploadWindow(){}

public:
  // Message handlers
  long onCmdBitrateListSlected(FXObject*, FXSelector, void*);
  long onCmdFrameListSlected(FXObject*, FXSelector, void*);
  long onCmdFrameDetailListSlected(FXObject*, FXSelector, void*);
  long onMouseUp(FXObject*, FXSelector, void*);
  long onMouseMove(FXObject*, FXSelector, void*);
  long onExpose(FXObject*, FXSelector, void*);
  long onConfigure(FXObject*, FXSelector, void*);

  long onTimeout(FXObject*, FXSelector, void*);
  long onChore(FXObject*, FXSelector, void*);
  long OnCanvasUpdate(FXObject*, FXSelector, void*);

  long onCmdCamera(FXObject*, FXSelector, void*);
  long onCmdAudio(FXObject*, FXSelector, void*);

  long onCmdCreateStream(FXObject*, FXSelector, void*);
  //long onCmdUploadInit(FXObject*, FXSelector, void*);
  long onCmdUploadStart(FXObject*, FXSelector, void*);
  long onCmdAutoUpload(FXObject*, FXSelector, void*);
  long onCmdLostRate(FXObject* sender, FXSelector, void*);
  long onCmdFrameRate(FXObject* sender, FXSelector, void*);

  long onCmdFecCheck(FXObject* sender, FXSelector, void*);
  long onCmdNackCheck(FXObject* sender, FXSelector, void*);
  long onCmdAfpsCheck(FXObject* sender, FXSelector, void*);
  long onCmdChangeNet(FXObject* sender, FXSelector, void*);

  long onCmdLogLevelChanged(FXObject*, FXSelector, void*);
  long onCmdProtocolChanged(FXObject*, FXSelector, void*);
  long onCmdReportChanged(FXObject*, FXSelector, void*);

  long onCmdCheckAuto(FXObject*, FXSelector, void*);

  long onCmdPlay(FXObject* sender, FXSelector, void*);
  long onCmdCameraChanged(FXObject*, FXSelector, void*);

public:
  void drawScene();
  void drawCamera(int canvaswidth, int canvasheight);

  void Update();

private:

  void UpdateSendStatus();

  void LoadCamera();
  void LoadResolution(int camera_index);
  void LoadAudio();

  void CalcCanvasSize(int panelwd, int panelht, int& canvaswd, int& canvasht);

  static void PreviewVideoCallback(void *captureCtx, char* data[3], lfrtcRawVideoType type, int width, int height);
  void PreviewVideoCallbackImpl(char* data[3], lfrtcRawVideoType type, int width, int height);

private:
  FXGLCanvas    *glcanvas;      // GL Canvas to draw into
  FXGLVisual    *glvisual;      // OpenGL visual
  FXGroupBox	  *streamgp;
  FXGroupBox	  *addrgp;

  FXCheckButton *checkBtn;

  FXToggleButton *camera_btn_;
  FXToggleButton *audio_btn_;
  FXToggleButton *upload_start_btn_;
  FXToggleButton *autolive_btn_;

  FXButton	  *opencamera_btn_;
  FXButton	  *closecamera_btn_;
  FXButton	  *openaudio_btn_;
  FXButton	  *beginlive_btn_;
  FXButton	  *stoplive_btn_;
  FXButton	  *createstream_btn_;
  FXButton	  *upload_init_btn_;
  FXButton	  *requestaddr_btn_;

  FXTextField	  *appid_text_;
  FXTextField	  *alias_text_;
  FXTextField	  *streamid_text_;
  FXTextField	  *ip_text_;
  FXTextField	  *tcp_port_text_;
  FXTextField	  *udp_port_text_;
  FXTextField	  *http_port_text_;
  FXTextField	  *lost_rate_text_;
  FXTextField	  *frame_rate_text_;
  FXTextField	  *log_size_text_;
  FXTextField	  *video_rate_text_;
  FXTextField	  *audio_rate_text_;

  FXLabel       *upload_status_label_;
  FXList        *upload_bitrate_list_;
  FXList        *upload_detail_frame_list_;
  FXList        *upload_frame_list_;

  FXCheckButton *fec_checkbutton_;
  FXCheckButton *nack_checkbutton_;
  FXCheckButton *afps_checkbutton_;//adapt fps
  FXButton *netchange_checkbutton_;


  FXComboBox	  *camera_combo_;
  FXComboBox	  *res_combo_;
  FXComboBox	  *audio_combo_;
  FXComboBox    *lapi_combo_;
  FXComboBox    *report_combo_;

  FXComboBox    *loglevel_combo_;
  FXComboBox    *protocol_combo_;

  FXdouble       rts;
  FXTime         lasttime;
  int            spinning;      // Is box spinning
  int            opening;
  int			   audio_opening_;
  double         angle;         // Rotation angle of box
  //FXDataTarget   dt_rts;	

  int camera_wd_;
  int camera_ht_;

  int           is_bitrate_auto_down_;
  int           is_frame_auto_down_;
  int           is_frame_datail_auto_down_;
  
  void *preview_window_;
  lfrtcDevice camera_devices_[5];
  lfrtcDevice audio_devices_[5];
  lfrtcCameraCapability camera_capability_[20];
  CRITICAL_SECTION cs_;
  char *preview_buf_;
  int preview_buf_len_;

  RtcCapture *capture_;
};
