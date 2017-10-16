#include "upload.h"

#include "../rtpplayer_test_base.h"
#include "engine_api/rtp_upload.h"
#include "engine_api/rtp_api.h"
#include "engine_api/RtcLog.h"
#include <vector>
#include <string.h>
#include <objbase.h>
#include <Iphlpapi.h>
#include <windows.h>
#include <atlbase.h>
#include <atlwin.h>

CComModule _Module;

#pragma warning(disable: 4800)

namespace {
  std::wstring utf82unicode(const char* s) {
    if (NULL == s) {
      return L"";
    }
    int nLen = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
    if (nLen <= 0) {
      return L"";
    }
    std::vector<wchar_t> szDst(nLen + 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, s, -1, &szDst[0], nLen);
    return &szDst[0];
  }

  void PrintMACaddress(unsigned char MACData[], char *mac) {
    sprintf(mac, "%02X-%02X-%02X-%02X-%02X-%02X",
      MACData[0], MACData[1], MACData[2], MACData[3], MACData[4], MACData[5]);
  }

  void GetMacAddress(char *mac) {
    IP_ADAPTER_INFO AdapterInfo[16];
    DWORD dwBuflen = sizeof(AdapterInfo);
    DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBuflen);
    PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
    do{
      PrintMACaddress(pAdapterInfo->Address, mac);
      pAdapterInfo = pAdapterInfo->Next;
    } while (0);//while (pAdapterInfo);
  }

  void UploaderCallback(RtcCapture* ctx, unsigned int msgid, int wParam, int lParam) {
    char msg[128];
    sprintf(msg, ".......................uploader callback, msgid=%u, wparam=%d, lparam=%d\n", msgid, wParam, lParam);
    OutputDebugStringA(msg);
  }
}

using namespace live_stream_sdk;
// Message Map
FXDEFMAP(UploadWindow) UploadWindowMap[] = {

  //________Message_Type_________ID_____________________Message_Handler_______
  FXMAPFUNC(SEL_PAINT, UploadWindow::ID_CANVAS, UploadWindow::onExpose),
  FXMAPFUNC(SEL_CONFIGURE, UploadWindow::ID_CANVAS, UploadWindow::onConfigure),
  FXMAPFUNC(SEL_TIMEOUT, UploadWindow::ID_TIMEOUT, UploadWindow::onTimeout),
  FXMAPFUNC(SEL_CHORE, UploadWindow::ID_CHORE, UploadWindow::onChore),
  FXMAPFUNC(SEL_UPDATE, UploadWindow::ID_CANVAS, UploadWindow::OnCanvasUpdate),
  FXMAPFUNC(SEL_COMMAND, UploadWindow::ID_CAMERA, UploadWindow::onCmdCamera),
  FXMAPFUNC(SEL_COMMAND, UploadWindow::ID_CREATE_STREAM, UploadWindow::onCmdCreateStream),
  //FXMAPFUNC(SEL_COMMAND, UploadWindow::ID_UPLOAD_INIT, UploadWindow::onCmdUploadInit),
  FXMAPFUNC(SEL_COMMAND, UploadWindow::ID_UPLOAD_START, UploadWindow::onCmdUploadStart),
  FXMAPFUNC(SEL_COMMAND, UploadWindow::ID_AUTO_UPLOAD, UploadWindow::onCmdAutoUpload),
  FXMAPFUNC(SEL_COMMAND, UploadWindow::ID_COMBO_LOGLEVEL, UploadWindow::onCmdLogLevelChanged),
  FXMAPFUNC(SEL_COMMAND, UploadWindow::ID_COMBO_PROTOCAL, UploadWindow::onCmdProtocolChanged),
  FXMAPFUNC(SEL_COMMAND, UploadWindow::ID_COMBO_REPORT, UploadWindow::onCmdReportChanged),
  FXMAPFUNC(SEL_COMMAND, UploadWindow::ID_CHECK_AUTO, UploadWindow::onCmdCheckAuto),
  FXMAPFUNC(SEL_COMMAND, UploadWindow::ID_COMBO_CAMERA, UploadWindow::onCmdCameraChanged),
  FXMAPFUNC(SEL_SELECTED, UploadWindow::ID_BITRATE_LIST, UploadWindow::onCmdBitrateListSlected),
  FXMAPFUNC(SEL_SELECTED, UploadWindow::ID_FRAME_LIST, UploadWindow::onCmdFrameListSlected),
  FXMAPFUNC(SEL_SELECTED, UploadWindow::ID_FRAMEDETAIL_LIST, UploadWindow::onCmdFrameDetailListSlected),

  FXMAPFUNC(SEL_COMMAND, UploadWindow::ID_LOST_RATE, UploadWindow::onCmdLostRate),
  FXMAPFUNC(SEL_COMMAND, UploadWindow::ID_FRAME_RATE, UploadWindow::onCmdFrameRate),
  FXMAPFUNC(SEL_COMMAND, UploadWindow::ID_ENABLE_FEC, UploadWindow::onCmdFecCheck),
  FXMAPFUNC(SEL_COMMAND, UploadWindow::ID_ENABLE_NACK, UploadWindow::onCmdNackCheck),
  FXMAPFUNC(SEL_COMMAND, UploadWindow::ID_ENABLE_AFPS, UploadWindow::onCmdAfpsCheck),
  FXMAPFUNC(SEL_COMMAND, UploadWindow::ID_ENABLE_NETCHANGE, UploadWindow::onCmdChangeNet),

  FXMAPFUNC(SEL_COMMAND, UploadWindow::ID_PLAYER, UploadWindow::onCmdPlay),
};

// Implementation
FXIMPLEMENT(UploadWindow, FXMainWindow, UploadWindowMap, ARRAYNUMBER(UploadWindowMap))

// Construct a GLTestApp
UploadWindow::UploadWindow(FXApp* a) : FXMainWindow(a, "upload_unitest", NULL, NULL, DECOR_ALL, 0, 0, 960, 740){
  CoInitialize(nullptr);

  InitializeCriticalSection(&cs_);

  char mac_address[256] = { 0 };
  GetMacAddress(mac_address);
  capture_ = new RtcCapture(mac_address);
  capture_->SetUserdata(this);

  preview_buf_ = NULL;
  preview_buf_len_ = 0;

  FXVerticalFrame *glcanvasFrame;
  FXVerticalFrame *rightFrame;
  FXVerticalFrame *bottomFrame;
  FXComposite *glpanel;

  FXDockSite *rightdock = new FXDockSite(this, LAYOUT_SIDE_RIGHT | LAYOUT_FILL_Y);
  FXDockSite *bottomdock = new FXDockSite(this, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X);

  FXToolBarShell * rightdragshell = new FXToolBarShell(this, LAYOUT_FILL_Y | FRAME_RAISED | FRAME_THICK, 0, 0, 0, 0, 10, 10);
  FXDockBar *rightdockbar = new FXDockBar(rightdock, rightdragshell, LAYOUT_FIX_WIDTH | LAYOUT_FILL_Y | LAYOUT_SIDE_RIGHT, 0, 0, 300, 0, 2, 2, 2, 2, 2, 2);
  rightdockbar->allowedSides(FXDockBar::ALLOW_LEFT | FXDockBar::ALLOW_RIGHT);

  FXHorizontalFrame *rightdockframe = new FXHorizontalFrame(rightdockbar, LAYOUT_SIDE_TOP | LAYOUT_FILL_X, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  new FXDockTitle(rightdockframe, "Controls", rightdockbar, FXToolBar::ID_TOOLBARGRIP, LAYOUT_FILL_X | FRAME_SUNKEN | JUSTIFY_CENTER_X);
  new FXMDIDeleteButton(rightdockframe, rightdockbar, FXWindow::ID_HIDE, LAYOUT_FILL_Y);

  FXTabBook* panels = new FXTabBook(rightdockbar, NULL, 0, LAYOUT_FILL_Y | LAYOUT_FILL_X, 0, 0, 0, 0, 0, 0, 0, 0);

  new FXTabItem(panels, "Camera\tCamera Camera\tConfig camera panel.");

  // RIGHT pane for the buttons
  rightFrame = new FXVerticalFrame(panels, LAYOUT_SIDE_RIGHT | LAYOUT_FIX_WIDTH, 0, 0, 240, 0, 2, 2, 3, 3);

  // LEFT pane to contain the glcanvas
  glcanvasFrame = new FXVerticalFrame(this, LAYOUT_FILL_X | LAYOUT_FILL_Y, 0, 0, 0, 0, 2, 2, 3, 3);

  // Drawing glcanvas
  glpanel = new FXVerticalFrame(glcanvasFrame, FRAME_SUNKEN | FRAME_THICK | LAYOUT_FILL_X | LAYOUT_FILL_Y | LAYOUT_CENTER_Y | LAYOUT_LEFT, 0, 0, 0, 0, 0, 0, 0, 0);
  //glpanel->setBackColor(FXRGB(128, 128, 128));
  // A Visual to drag OpenGL


  //FXList *testlist = new FXList(glcanvasFrame, NULL, 0, LIST_EXTENDEDSELECT | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT, 0, 0, 0, 100); //add by 

  glvisual = new FXGLVisual(getApp(), VISUAL_DOUBLE_BUFFER);

  // Drawing glcanvas
  glcanvas = new FXGLCanvas(glpanel, glvisual, this, ID_CANVAS, LAYOUT_FIX_HEIGHT | LAYOUT_FIX_WIDTH | LAYOUT_CENTER_X | LAYOUT_CENTER_Y, 0, 0, 640, 480);

  //glcanvas->setBackColor(FXRGB(128, 128, 128));

  FXGroupBox *cameragp = new FXGroupBox(rightFrame, "", LAYOUT_SIDE_TOP | FRAME_GROOVE | LAYOUT_FILL_X, 0, 0, 0, 0);

  camera_combo_ = new FXComboBox(cameragp, 5, this, ID_COMBO_CAMERA, COMBOBOX_STATIC | COMBOBOX_INSERT_LAST | FRAME_SUNKEN | FRAME_THICK | LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
  camera_combo_->setNumVisible(5);

  res_combo_ = new FXComboBox(cameragp, 5, this, ID_COMBO_RES, COMBOBOX_STATIC | COMBOBOX_INSERT_LAST | FRAME_SUNKEN | FRAME_THICK | LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
  res_combo_->setNumVisible(5);

  audio_combo_ = new FXComboBox(cameragp, 5, this, ID_COMBO_AUDIO, COMBOBOX_STATIC | COMBOBOX_INSERT_LAST | FRAME_SUNKEN | FRAME_THICK | LAYOUT_SIDE_TOP);
  audio_combo_->setNumVisible(5);

  camera_btn_ = new FXToggleButton(cameragp, L"Begin Capture\tBeginCapture", L"Stop Capture\tStopCapture", NULL, NULL, this, ID_CAMERA, ICON_BEFORE_TEXT | FRAME_RAISED | FRAME_THICK | LAYOUT_FILL_X);

  ////////no-use////////
  FXGroupBox *audiogp = new FXGroupBox(rightFrame, "", LAYOUT_SIDE_TOP | FRAME_GROOVE | LAYOUT_FILL_X, 0, 0, 0, 0);

  audio_btn_ = new FXToggleButton(audiogp, L"Open Audio\tOpenAudio", L"Close Audio\tCloseAudio", NULL, NULL, this, ID_AUDIO, ICON_BEFORE_TEXT | FRAME_RAISED | FRAME_THICK | LAYOUT_FILL_X);
  audio_btn_->hide();
  openaudio_btn_ = new FXButton(audiogp, tr("&OpenAudio\tOpenAudio"), NULL, this, ID_OPEN_AUDIO, FRAME_THICK | FRAME_RAISED | LAYOUT_FILL_X | LAYOUT_TOP | LAYOUT_LEFT, 0, 0, 0, 0, 10, 10, 5, 5);
  openaudio_btn_->hide();

  audiogp->hide();

  FXMatrix* aliasmatrix = new FXMatrix(rightFrame, 4, MATRIX_BY_COLUMNS | LAYOUT_FILL_X);
  new FXLabel(aliasmatrix, "Lapi", NULL, LAYOUT_LEFT | FRAME_NONE);

  lapi_combo_ = new FXComboBox(aliasmatrix, 5, this, ID_COMBO_LAPI, COMBOBOX_STATIC | COMBOBOX_INSERT_LAST | FRAME_SUNKEN | FRAME_THICK | LAYOUT_RIGHT | LAYOUT_FIX_WIDTH, 0, 0, 120, 0);
  lapi_combo_->setNumVisible(5);

  //set lapi list
  {
    lapi_combo_->appendItem(tr("101.201.57.242"));
    lapi_combo_->appendItem(tr("lapi.xiu.youku.com"));
    lapi_combo_->appendItem(tr("101.200.47.145"));// lapi.xiu.youku.com    
    lapi_combo_->appendItem(tr("103.41.143.105"));
  }


  new FXLabel(aliasmatrix, "Appid", NULL, LAYOUT_LEFT | FRAME_NONE);
  appid_text_ = new FXTextField(aliasmatrix, 0, NULL, 0, FRAME_SUNKEN | FRAME_THICK | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_COLUMN);
  appid_text_->setText(tr("301"));

  new FXLabel(aliasmatrix, "Alias", NULL, LAYOUT_LEFT | FRAME_NONE);
  alias_text_ = new FXTextField(aliasmatrix, 2, NULL, 0, FRAME_SUNKEN | FRAME_THICK | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_COLUMN);
  alias_text_->setText(tr("zhangle"));

  createstream_btn_ = new FXButton(rightFrame, L"CreateStream\tCreateStream", NULL, this, ID_CREATE_STREAM, FRAME_THICK | FRAME_RAISED | LAYOUT_FILL_X  /*| LAYOUT_TOP | LAYOUT_LEFT*/, 0, 0, 0, 0, 10, 10, 5, 5);
  FXMatrix* streammatrix = new FXMatrix(rightFrame, 2, MATRIX_BY_COLUMNS | LAYOUT_FILL_X);
  streamid_text_ = new FXTextField(rightFrame, 2, NULL, 0, FRAME_SUNKEN | FRAME_THICK | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_COLUMN);
  //streamid_text_->setText(tr("000000000000000000000000156f1537"));

  FXGroupBox *autolivegp = new FXGroupBox(rightFrame, L"Auto", LAYOUT_SIDE_TOP | FRAME_GROOVE | LAYOUT_FILL_X, 0, 0, 0, 0);
  autolive_btn_ = new FXToggleButton(autolivegp, L"Auto Begin Upload\tBeginLive", L"Stop Upload\tStopLive", NULL, NULL, this, ID_AUTO_UPLOAD, ICON_BEFORE_TEXT | FRAME_RAISED | FRAME_THICK | LAYOUT_FILL_X);

  FXGroupBox *manual_livegp = new FXGroupBox(rightFrame, L"Manual", LAYOUT_SIDE_TOP | FRAME_GROOVE | LAYOUT_FILL_X, 0, 0, 0, 0);

  upload_init_btn_ = new FXButton(manual_livegp, L"GetReceiver\tUploadInit", NULL, this, ID_UPLOAD_INIT, FRAME_THICK | FRAME_RAISED | LAYOUT_FILL_X /*| LAYOUT_TOP | LAYOUT_LEFT*/, 0, 0, 0, 0, 10, 10, 5, 5);
  upload_init_btn_->hide();
  FXMatrix* upload_init_matrix = new FXMatrix(manual_livegp, 4, MATRIX_BY_COLUMNS | LAYOUT_FILL_X);

  new FXLabel(upload_init_matrix, "ip", NULL, LAYOUT_LEFT | FRAME_NONE);
  ip_text_ = new FXTextField(upload_init_matrix, 2, NULL, 0, FRAME_SUNKEN | FRAME_THICK | LAYOUT_SIDE_TOP | LAYOUT_FIX_WIDTH | LAYOUT_FILL_COLUMN, 0, 0, 100, 0);
  ip_text_->setText(tr("192.168.245.133"));

  new FXLabel(upload_init_matrix, "udp", NULL, LAYOUT_LEFT | FRAME_NONE);
  udp_port_text_ = new FXTextField(upload_init_matrix, 2, NULL, 0, FRAME_SUNKEN | FRAME_THICK | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_COLUMN);
  udp_port_text_->setText(tr("8142"));

  new FXLabel(upload_init_matrix, "tcp", NULL, LAYOUT_LEFT | FRAME_NONE);
  tcp_port_text_ = new FXTextField(upload_init_matrix, 2, NULL, 0, FRAME_SUNKEN | FRAME_THICK | LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
  tcp_port_text_->setText(tr("8102"));

  new FXLabel(upload_init_matrix, "http", NULL, LAYOUT_LEFT | FRAME_NONE);
  http_port_text_ = new FXTextField(upload_init_matrix, 2, NULL, 0, FRAME_SUNKEN | FRAME_THICK | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_COLUMN);
  http_port_text_->setText(tr("8142"));

  new FXLabel(upload_init_matrix, "videoRate", NULL, LAYOUT_LEFT | FRAME_NONE);
  video_rate_text_ = new FXTextField(upload_init_matrix, 2, NULL, 0, FRAME_SUNKEN | FRAME_THICK | LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
  video_rate_text_->setText(tr("1000"));

  upload_start_btn_ = new FXToggleButton(manual_livegp, L"ManualUpload\tBeginLive", L"Stop Upload\tStopLive", NULL, NULL, this, ID_UPLOAD_START, ICON_BEFORE_TEXT | FRAME_RAISED | FRAME_THICK | LAYOUT_FILL_X);

  FXMatrix* config_matrix = new FXMatrix(rightFrame, 4, MATRIX_BY_COLUMNS | LAYOUT_FILL_X);
  FXButton* lost_rate_btn = new FXButton(config_matrix, L"LostRate", NULL, this, ID_LOST_RATE, FRAME_THICK | FRAME_RAISED | LAYOUT_FILL_X /*| LAYOUT_TOP | LAYOUT_LEFT*/, 0, 0, 0, 0, 10, 10, 5, 5);
  lost_rate_text_ = new FXTextField(config_matrix, 2, NULL, 0, FRAME_SUNKEN | FRAME_THICK | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_COLUMN);
  lost_rate_text_->setText(tr("0"));

  FXButton* frame_rate_btn = new FXButton(config_matrix, L"Fps", NULL, this, ID_FRAME_RATE, FRAME_THICK | FRAME_RAISED | LAYOUT_FILL_X /*| LAYOUT_TOP | LAYOUT_LEFT*/, 0, 0, 0, 0, 10, 10, 5, 5);
  frame_rate_text_ = new FXTextField(config_matrix, 2, NULL, 0, FRAME_SUNKEN | FRAME_THICK | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_COLUMN);
  frame_rate_text_->setText(tr("15"));

  new FXLabel(config_matrix, "LogLevel", NULL, LAYOUT_LEFT | FRAME_NONE);

  loglevel_combo_ = new FXComboBox(config_matrix, 5, this, ID_COMBO_LOGLEVEL, COMBOBOX_STATIC | COMBOBOX_INSERT_LAST | FRAME_SUNKEN | FRAME_THICK | LAYOUT_RIGHT | LAYOUT_FILL_X, 0, 0, 0, 0);
  loglevel_combo_->setNumVisible(9);

  //set log level
  {
    loglevel_combo_->appendItem(tr("TRC"));
    loglevel_combo_->appendItem(tr("RTP"));
    loglevel_combo_->appendItem(tr("RTCP"));
    loglevel_combo_->appendItem(tr("RECOVER"));
    loglevel_combo_->appendItem(tr("DBG"));
    loglevel_combo_->appendItem(tr("INF"));
    loglevel_combo_->appendItem(tr("WRN"));
    loglevel_combo_->appendItem(tr("ERR"));
    loglevel_combo_->appendItem(tr("NON"));
  }
  loglevel_combo_->setCurrentItem(5);

  new FXLabel(config_matrix, "Protocol", NULL, LAYOUT_LEFT | FRAME_NONE);

  protocol_combo_ = new FXComboBox(config_matrix, 5, this, ID_COMBO_PROTOCAL, COMBOBOX_STATIC | COMBOBOX_INSERT_LAST | FRAME_SUNKEN | FRAME_THICK | LAYOUT_RIGHT | LAYOUT_FILL_X, 0, 0, 0, 0);
  protocol_combo_->setNumVisible(2);
  //set protocol
  {
    protocol_combo_->appendItem(tr("TCP"));
    protocol_combo_->appendItem(tr("UDP"));
  }
  protocol_combo_->setCurrentItem(1);


  FXMatrix* config_matrix_checkbutton = new FXMatrix(rightFrame, 4, MATRIX_BY_COLUMNS | LAYOUT_FILL_X);
  fec_checkbutton_ = new FXCheckButton(config_matrix_checkbutton, L"FEC", this, ID_ENABLE_FEC, ICON_BEFORE_TEXT /*| LAYOUT_TOP | LAYOUT_LEFT*/, 0, 0, 0, 0, 2, 2, 2, 2);
  //fec_checkbutton_->setCheck();

  nack_checkbutton_ = new FXCheckButton(config_matrix_checkbutton, L"NACK", this, ID_ENABLE_NACK, ICON_BEFORE_TEXT /*| LAYOUT_TOP | LAYOUT_LEFT*/, 0, 0, 0, 0, 2, 2, 2, 2);
  nack_checkbutton_->setCheck();

  afps_checkbutton_ = new FXCheckButton(config_matrix_checkbutton, L"AFPS", this, ID_ENABLE_AFPS, ICON_BEFORE_TEXT /*| LAYOUT_TOP | LAYOUT_LEFT*/, 0, 0, 0, 0, 2, 2, 2, 2);

  netchange_checkbutton_ = new FXButton(config_matrix_checkbutton, L"ChangeNet", NULL, this, ID_ENABLE_NETCHANGE, FRAME_THICK | FRAME_RAISED | LAYOUT_FILL_X /*| LAYOUT_TOP | LAYOUT_LEFT*/, 0, 0, 0, 0, 2, 2, 2, 2);
  //netchange_checkbutton_->setCheck();

  upload_status_label_ = new FXLabel(rightFrame, "Status: Ready to upload", NULL, LAYOUT_LEFT | FRAME_NONE);

  FXMatrix* player_matrix_ = new FXMatrix(rightFrame, 4, MATRIX_BY_COLUMNS | LAYOUT_FILL_X);

  FXToggleButton* player_btn_ = new FXToggleButton(player_matrix_, L"Start Play", L"Stop Play", NULL, NULL, this, ID_PLAYER, ICON_BEFORE_TEXT | FRAME_RAISED | FRAME_THICK | LAYOUT_FILL_X);
  //lost_rate_text_ = new FXTextField(config_matrix, 2, NULL, 0, FRAME_SUNKEN | FRAME_THICK | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_COLUMN);
  //lost_rate_text_->setText(tr("0"));

  // Exit button
  new FXButton(rightFrame, tr("&Exit\tExit the application"), NULL, getApp(), FXApp::ID_QUIT, FRAME_THICK | FRAME_RAISED | LAYOUT_FILL_X | LAYOUT_BOTTOM | LAYOUT_LEFT, 0, 0, 0, 0, 10, 10, 5, 5);

  //bottom
  FXToolBarShell * bottomdragshell = new FXToolBarShell(this, LAYOUT_FILL_X | FRAME_RAISED | FRAME_THICK, 0, 0, 0, 0, 10, 10);
  FXDockBar *bottomdockbar = new FXDockBar(bottomdock, bottomdragshell, LAYOUT_FILL_X | LAYOUT_SIDE_BOTTOM, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2);
  bottomdockbar->allowedSides(FXDockBar::ALLOW_BOTTOM);

  FXHorizontalFrame *bottomdockframe = new FXHorizontalFrame(bottomdockbar, LAYOUT_SIDE_TOP | LAYOUT_FILL_X, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  new FXDockTitle(bottomdockframe, "Logs", bottomdockbar, FXToolBar::ID_TOOLBARGRIP, LAYOUT_FILL_X | FRAME_SUNKEN | JUSTIFY_CENTER_X);
  new FXMDIDeleteButton(bottomdockframe, bottomdockbar, FXWindow::ID_HIDE, LAYOUT_FILL_Y);

  FXTabBook* bottompanels = new FXTabBook(bottomdockbar, NULL, 0, LAYOUT_FILL_Y | LAYOUT_FILL_X, 0, 0, 0, 0, 0, 0, 0, 0);

  new FXTabItem(bottompanels, "Upload\tUpload logs\tUpload logs panel.");

  bottomFrame = new FXVerticalFrame(bottompanels, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT, 0, 0, 0, 200, 2, 2, 3, 3);

  upload_bitrate_list_ = new FXList(bottomFrame, this, ID_BITRATE_LIST, LIST_EXTENDEDSELECT | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT, 0, 0, 0, 100);

  new FXTabItem(bottompanels, "Frame\tFrame logs\tFrame logs panel.");
  FXVerticalFrame *bottomFrame2 = new FXVerticalFrame(bottompanels, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT, 0, 0, 0, 200, 2, 2, 3, 3);

  upload_frame_list_ = new FXList(bottomFrame2, this, ID_FRAME_LIST, LIST_EXTENDEDSELECT | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT, 0, 0, 0, 100);

  new FXTabItem(bottompanels, "FrameDetail\tFrameDetail\tFrameDetail.");
  FXVerticalFrame *bottomFrame3 = new FXVerticalFrame(bottompanels, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT, 0, 0, 0, 200, 2, 2, 3, 3);

  upload_detail_frame_list_ = new FXList(bottomFrame3, this, ID_FRAMEDETAIL_LIST, LIST_EXTENDEDSELECT | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT, 0, 0, 0, 100);

  // Make a tooltip
  new FXToolTip(getApp());

  // Initialize private variables
  spinning = 0;
  opening = 0;
  audio_opening_ = 0;
  angle = 0.0;
  rts = 1.0;
  //dt_rts.connect(rts);

  camera_wd_ = 0;
  camera_ht_ = 0;

  LoadCamera(); //test
  LoadAudio();

  is_bitrate_auto_down_ = 0;
  is_frame_auto_down_ = 0;
  is_frame_datail_auto_down_ = 0;
}

// Destructor
UploadWindow::~UploadWindow(){
  getApp()->removeTimeout(this, ID_TIMEOUT);
  getApp()->removeChore(this, ID_CHORE);

  INF("UploadWindow destroyed");
  delete glvisual;      // OpenGL visual	
  delete capture_;

  DeleteCriticalSection(&cs_);
  CoUninitialize();
}

// Create and initialize
void UploadWindow::create(){
  FXMainWindow::create();
  show(PLACEMENT_SCREEN);
}

void UploadWindow::destroy() {
  capture_->Stop();
  FXMainWindow::destroy();
}

long UploadWindow::onCmdBitrateListSlected(FXObject*, FXSelector, void*) {
  static int pre_index = -1;
  int index = upload_bitrate_list_->getCurrentItem();
  if (pre_index == index)
  {
    is_bitrate_auto_down_ = 0;
    upload_bitrate_list_->deselectItem(index);
  }
  else
  {
    is_bitrate_auto_down_ = 1;
  }
  pre_index = index;
  return 1;
}

long UploadWindow::onCmdFrameListSlected(FXObject*, FXSelector, void*) {
  static int pre_index = -1;
  int index = upload_frame_list_->getCurrentItem();
  if (pre_index == index)
  {
    is_frame_auto_down_ = 0;
    upload_frame_list_->deselectItem(index);
  }
  else
  {
    is_frame_auto_down_ = 1;
  }
  pre_index = index;
  return 1;
}

long UploadWindow::onCmdFrameDetailListSlected(FXObject*, FXSelector, void*) {
  static int pre_index = -1;
  int index = upload_detail_frame_list_->getCurrentItem();
  if (pre_index == index)
  {
    is_frame_datail_auto_down_ = 0;
    upload_detail_frame_list_->deselectItem(index);
  }
  else
  {
    is_frame_datail_auto_down_ = 1;
  }
  pre_index = index;
  return 1;
}

// Widget has been resized
long UploadWindow::onConfigure(FXObject*, FXSelector, void*){
  if (glcanvas->makeCurrent()){
    glViewport(0, 0, glcanvas->getWidth(), glcanvas->getHeight());
    glcanvas->makeNonCurrent();
  }
  return 1;
}

// Widget needs repainting
long UploadWindow::onExpose(FXObject*, FXSelector, void*){
  drawScene();
  return 1;
}

// Rotate the boxes when a timer message is received
long UploadWindow::onTimeout(FXObject*, FXSelector, void*){
  UpdateSendStatus();
  getApp()->addTimeout(this, ID_TIMEOUT, TIMER_INTERVAL);
  return 1;
}

// Rotate the boxes when a chore message is received
long UploadWindow::onChore(FXObject*, FXSelector, void*){
  FXTime c = FXThread::time();
  FXTime d = c - lasttime;
  angle += (d / 1000000000.0)*(360.0*rts);
  if (angle > 360.0) angle -= 360.0;
  lasttime = c;
  drawScene();
  getApp()->addChore(this, ID_CHORE);
  return 1;
}

long UploadWindow::OnCanvasUpdate(FXObject*, FXSelector, void*){
  drawScene();
  return 1;
}

// Draw the GL scene
void UploadWindow::drawScene(){
  const GLfloat lightPosition[] = { 15., 10., 5., 1. };
  const GLfloat lightAmbient[] = { .1f, .1f, .1f, 1. };
  const GLfloat lightDiffuse[] = { .9f, .9f, .9f, 1. };
  const GLfloat redMaterial[] = { 1., 0., 0., 1. };
  const GLfloat blueMaterial[] = { 0., 0., 1., 1. };

  GLdouble canvaswidth = glcanvas->getWidth();
  GLdouble canvasheight = glcanvas->getHeight();
  GLdouble aspect = canvasheight > 0 ? canvaswidth / canvasheight : 1.0;

  // Make context current
  if (glcanvas->makeCurrent()){

    glViewport(0, 0, glcanvas->getWidth(), glcanvas->getHeight());

    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glDisable(GL_DITHER);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    /*gluPerspective(30., aspect, 1., 100.);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(5., 10., 15., 0., 0., 0., 0., 1., 0.);*/

    glShadeModel(GL_SMOOTH);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    glMaterialfv(GL_FRONT, GL_AMBIENT, blueMaterial);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, blueMaterial);

    EnterCriticalSection(&cs_);
    int canvaswd = 640;
    int canvasht = 480;
    CalcCanvasSize(640, 480, canvaswd, canvasht);
    drawCamera(canvaswd, canvasht);
    LeaveCriticalSection(&cs_);

    // Swap if it is double-buffered
    if (glvisual->isDoubleBuffer()){
      glcanvas->swapBuffers();
    }
    // Make context non-current
    glcanvas->makeNonCurrent();
  }
}

void UploadWindow::CalcCanvasSize(int panelwd, int panelht, int& canvaswd, int& canvasht) {
  int wd = 640;
  int ht = 480;

  if (camera_ht_ > 0 && camera_wd_ > 0) {
    if (panelwd*camera_ht_ >= panelht*camera_wd_) {
      ht = panelht;
      wd = panelht*camera_wd_ / camera_ht_;
    }
    else {
      wd = panelwd;
      ht = panelwd*camera_ht_ / camera_wd_;
    }
    canvaswd = wd;
    canvasht = ht;
  }
}

void UploadWindow::drawCamera(int canvaswidth, int canvasheight) {
  canvasheight = -canvasheight;
  if (preview_buf_ != NULL && camera_wd_ > 0 && camera_ht_ > 0) {
    GLdouble wdrate = (GLdouble)canvaswidth / camera_wd_;
    GLdouble htrate = (GLdouble)canvasheight / camera_ht_;

    int xoff = (640 - canvaswidth);
    int yoff = (480 - canvasheight);
    GLdouble xspect = (GLdouble)xoff / 640 - 1.0;
    GLdouble yspect = (GLdouble)yoff / 480 - 1.0;

    glRasterPos2f((GLfloat)xspect, (GLfloat)yspect);
    glDrawPixels(camera_wd_, camera_ht_, GL_BGR_EXT, GL_UNSIGNED_BYTE, preview_buf_);

    glPixelZoom((GLfloat)wdrate, (GLfloat)htrate);
  }
}

void UploadWindow::Update() {
  // glcanvas->forceRefresh();
  forceRefresh();
}

void UploadWindow::UpdateSendStatus() {
  {
    rtcUploadDispatchConfig dispatch;
    capture_->GetUploader()->GetDispatchConfig(&dispatch);
    ip_text_->setText(dispatch.mcu_ip);
    char temp[128];
    sprintf(temp, "%d", (int)dispatch.mcu_udp_port);
    udp_port_text_->setText(temp);
    sprintf(temp, "%d", (int)dispatch.mcu_tcp_port);
    tcp_port_text_->setText(temp);
    if (dispatch.sdp_url[0]) {
      int port = 80;
      char *pos = strrchr(dispatch.sdp_url, ':');
      if (pos) {
        port = atoi(pos + 1);
        if (port == 0) {
          port = 80;
        }
      }
      char tmp[32] = { 0 };
      sprintf(tmp, "%d", port);
      http_port_text_->setText(tmp);
    }
    if (streamid_text_->getText().empty()) {
      streamid_text_->setText(dispatch.streamid);
    }
  }

  RtcCaptureState net_state = capture_->GetUploader()->GetState();
  if (net_state == RTC_CAPTURE_STATE_INITIALIZING) {
    upload_status_label_->setText("Status: connecting");
  }
  else if (net_state == RTC_CAPTURE_STATE_STOPPED) {
    upload_status_label_->setText("Status: stopped");
  }
  else if (net_state == RTC_CAPTURE_STATE_ERROR) {
    upload_status_label_->setText("Status: upload failed");
  }
  else if (net_state == RTC_CAPTURE_STATE_RUNNING) {
    UploadNetworkState reporter;
    capture_->GetUploader()->GetNetworkState(&reporter);

    char temp[1024] = { 0 };
    char protocal[12];
    if (reporter.is_tcp) {
      sprintf(protocal, "TCP");
      protocol_combo_->setCurrentItem(0);
    }
    else {
      sprintf(protocal, "UDP");
      protocol_combo_->setCurrentItem(1);
    }
    RtcCapture::AdvancedStateInfo info;
    capture_->GetAdvancedStateInfo(&info);
    sprintf(temp, "target=%uk, encode=%dk, net=%dk, lost_rate=%u%%, rtt_ms=%d, fps=%d %s",
      info.video_target_bitrate / 1024, info.video_encode_bitrate / 1024,
      (reporter.video_bps + reporter.audio_bps) / 1024, reporter.packet_lost_percent, reporter.rtt_ms, info.video_encode_fps, protocal);

    upload_bitrate_list_->appendItem(temp);
    if (!is_bitrate_auto_down_)
      upload_bitrate_list_->makeItemVisible(upload_bitrate_list_->getNumItems() - 1);
    if (upload_bitrate_list_->getNumItems() > UPLOADBITRATENUM) {
      if (!is_bitrate_auto_down_)
      {
        for (int j = 0; j < upload_bitrate_list_->getNumItems() - UPLOADBITRATENUM; j++)
          upload_bitrate_list_->removeItem(0);
      }
      else if (upload_bitrate_list_->getNumItems() > 100 * UPLOADBITRATENUM)
      {
        int index = upload_bitrate_list_->getCurrentItem();
        is_bitrate_auto_down_ = 0;
        upload_bitrate_list_->deselectItem(index);
      }
    }

    upload_status_label_->setText("Status: upload success");
  }
}

void UploadWindow::PreviewVideoCallback(void *captureCtx, char* data[3], lfrtcRawVideoType type, int width, int height) {
  RtcCapture* capture = (RtcCapture*)captureCtx;
  UploadWindow* pThis = (UploadWindow*)capture->GetUserdata();
  pThis->PreviewVideoCallbackImpl(data, type, width, height);
}

void UploadWindow::PreviewVideoCallbackImpl(char* data[3], lfrtcRawVideoType type, int width, int height) {
  if (type == klfrtcVideoRGB24 && width > 0 && height > 0) {
    EnterCriticalSection(&cs_);
    camera_wd_ = width;
    camera_ht_ = height;
    if (preview_buf_ == NULL || preview_buf_len_ < width * height * 3) {
      delete[] preview_buf_;
      preview_buf_len_ = width * height * 3;
      preview_buf_ = new char[preview_buf_len_];
    }
    memcpy(preview_buf_, data[0], width * height * 3);
    LeaveCriticalSection(&cs_);
    Update();
  }
}

long UploadWindow::onCmdCamera(FXObject* sender, FXSelector, void*) {
  FXToggleButton* button = (FXToggleButton*)sender;
  FXuchar state = button->getState();
  if (state == 1) {
    FXint cameraindex = camera_combo_->getCurrentItem();
    FXint resindex = res_combo_->getCurrentItem();
    FXint audioindex = audio_combo_->getCurrentItem();
    const char* video_device_id = "";
    const char* audio_device_id = "";
    if (cameraindex >= 0 && cameraindex < _countof(camera_devices_)) {
      video_device_id = camera_devices_[cameraindex].szDeviceID;
    }
    if (audioindex >= 0 && audioindex < _countof(audio_devices_)) {
      audio_device_id = audio_devices_[audioindex].szDeviceID;
    }
    unsigned int width = 640;
    unsigned int height = 480;
    if (resindex >= 0 && resindex < _countof(camera_capability_) && camera_capability_[resindex].width > 0) {
      width = camera_capability_[resindex].width;
      height = camera_capability_[resindex].height;
    }

    lfrtcCaptureConfig config;
    strcpy(config.audio_deviceid, audio_device_id);
    strcpy(config.video_deviceid, video_device_id);
    config.video_capture_width = width;
    config.video_capture_height = height;
    capture_->StartCapture(&config);
    capture_->StartPreview(PreviewVideoCallback, klfrtcVideoRGB24);

    opening = 1;
    camera_combo_->disable();
    res_combo_->disable();
    audio_combo_->disable();
  }
  else {
    capture_->Stop();
    opening = 0;
    camera_combo_->enable();
    res_combo_->enable();
    audio_combo_->enable();
  }
  return 1;
}

long UploadWindow::onCmdCreateStream(FXObject*, FXSelector, void*) {
  FXString lapi_url = lapi_combo_->getText();
  FXString appid = appid_text_->getText();
  FXString alias = alias_text_->getText();

  char streamid[256] = { 0 };
  char url[1024];
  sprintf(url, "http://%s/v1/create_stream?app_id=%s&alias=%s&stream_type=rtp&res=%dx%d&rt=400&stream_format=rtp&nt=1&token=98765&p2p=0",
    lapi_url.text(), appid.text(), alias.text(), camera_wd_, camera_ht_);
  create_stream_sync(streamid, url);

  streamid_text_->setText(streamid);
  return 1;
}

long UploadWindow::onCmdUploadStart(FXObject* sender, FXSelector, void*) {
  FXToggleButton* button = (FXToggleButton*)sender;
  FXuchar state = button->getState();
  if (state == 1) {
    FXString lapi_url = lapi_combo_->getText();
    FXString appid = appid_text_->getText();
    FXString alias = alias_text_->getText();
    FXString stream_id = streamid_text_->getText();
    FXString upload_ip = ip_text_->getText();
    FXString upload_udp_port = udp_port_text_->getText();
    FXString upload_tcp_port = tcp_port_text_->getText();
    FXString upload_http_port = http_port_text_->getText();
    FXString video_rate = video_rate_text_->getText();

    RtcCapture::NetworkConfig net;
    strcpy(net.lapi, lapi_url.text());
    strcpy(net.appid, appid.text());
    strcpy(net.alias, alias.text());
    strcpy(net.token, "98765");
    lfrtcEncodeConfig encode;
    encode.video_bitrate = atoi(video_rate_text_->getText().text()) * 1024;
    int ret = capture_->StartEncodeAndSend(&net, &encode, UploaderCallback);
    if (ret < 0) {
      upload_status_label_->setText("capture_->StartEncodeAndSend failed.");
      return 0;
    }
    capture_->GetUploader()->Stop();

    rtcUploadDispatchConfig dispatch;
    strcpy(dispatch.appid, appid.text());
    strcpy(dispatch.alias, alias.text());
    strcpy(dispatch.lapi_host, lapi_url.text());
    strcpy(dispatch.lapi_token, "98765");
    dispatch.is_tcp = (protocol_combo_->getCurrentItem() == 0);
    strcpy(dispatch.streamid, stream_id.text());
    strcpy(dispatch.mcu_ip, upload_ip.text());
    dispatch.mcu_udp_port = (unsigned short)atoi(upload_udp_port.text());
    dispatch.mcu_tcp_port = (unsigned short)atoi(upload_tcp_port.text());
    strcpy(dispatch.mcu_token, "98765");
    sprintf(dispatch.sdp_url, "http://%s:%s/upload/sdp?streamid=%s", dispatch.mcu_ip, upload_http_port.text(), dispatch.streamid);
    capture_->GetUploader()->Start(&dispatch);

    INF("Begin to upload onCmdUploadStart");

    getApp()->addTimeout(this, ID_TIMEOUT, TIMER_INTERVAL);

    camera_combo_->disable();
    res_combo_->disable();
    audio_combo_->disable();
    lapi_combo_->disable();
    camera_btn_->disable();
    audio_btn_->disable();
    autolive_btn_->disable();
    createstream_btn_->disable();
    upload_init_btn_->disable();
  }
  else {
    getApp()->removeTimeout(this, ID_TIMEOUT);
    upload_status_label_->setText("Status: upload finished");
    capture_->StopEncodeAndSend();

    lapi_combo_->enable();
    camera_btn_->enable();
    audio_btn_->enable();
    autolive_btn_->enable();
    createstream_btn_->enable();
    upload_init_btn_->enable();
  }
  return 1;
}

long UploadWindow::onCmdAutoUpload(FXObject* sender, FXSelector, void*) {
  FXToggleButton* button = (FXToggleButton*)sender;
  FXuchar state = button->getState();
  if (state == 1) {
    FXString lapi_url = lapi_combo_->getText();
    FXString appid = appid_text_->getText();
    FXString alias = alias_text_->getText();

    INF("win upload starting");

    upload_status_label_->setText("Status: uploading");

    RtcCapture::NetworkConfig net;
    strcpy(net.lapi, lapi_url.text());
    strcpy(net.appid, appid.text());
    strcpy(net.alias, alias.text());
    strcpy(net.token, "98765");
    lfrtcEncodeConfig encode;
    encode.video_bitrate = atoi(video_rate_text_->getText().text()) * 1024;
    int ret = capture_->StartEncodeAndSend(&net, &encode, UploaderCallback);
    if (ret < 0) {
      upload_status_label_->setText("capture_->StartEncodeAndSend failed.");
      return 0;
    }

    getApp()->addTimeout(this, ID_TIMEOUT, TIMER_INTERVAL);

    camera_combo_->disable();
    res_combo_->disable();
    audio_combo_->disable();
    lapi_combo_->disable();
    camera_btn_->disable();
    audio_btn_->disable();
    upload_start_btn_->disable();
    createstream_btn_->disable();
    upload_init_btn_->disable();
  }
  else {
    getApp()->removeTimeout(this, ID_TIMEOUT);
    upload_status_label_->setText("Status: upload finished");
    capture_->StopEncodeAndSend();

    INF("win upload stopped");
    lapi_combo_->enable();
    camera_btn_->enable();
    audio_btn_->enable();
    upload_start_btn_->enable();
    createstream_btn_->enable();
    upload_init_btn_->enable();
  }
  return 1;
}

long UploadWindow::onCmdLostRate(FXObject* sender, FXSelector, void*) {
  FXString lost_rate = lost_rate_text_->getText();
  capture_->GetUploader()->DebugLost(lost_rate.toUInt(), lost_rate.toUInt());
  return 1;
}

long UploadWindow::onCmdFrameRate(FXObject* sender, FXSelector, void*) {
  //FXString frame_rate = frame_rate_text_->getText();
  //int fps = encode_man_.set_real_framerate(frame_rate.toUInt());
  //char temp[128];
  //sprintf(temp, "%d", fps);
  //frame_rate_text_->setText(temp);
  return 1;
}

long UploadWindow::onCmdFecCheck(FXObject* sender, FXSelector, void*) {
  FXCheckButton* checkbox = (FXCheckButton*)sender;
  FXuchar check = checkbox->getCheck();
  capture_->GetUploader()->EnableFec((bool)check);
  return 1;
}

long UploadWindow::onCmdNackCheck(FXObject* sender, FXSelector, void*) {
  FXCheckButton* checkbox = (FXCheckButton*)sender;
  FXuchar check = checkbox->getCheck();
  capture_->GetUploader()->EnableNack((bool)check);
  return 1;
}

long UploadWindow::onCmdAfpsCheck(FXObject* sender, FXSelector, void*) {
  FXCheckButton* checkbox = (FXCheckButton*)sender;
  FXuchar check = checkbox->getCheck();
  return 1;
}

long UploadWindow::onCmdChangeNet(FXObject* sender, FXSelector, void*) {
  capture_->GetUploader()->SetNetworkChanged();
  return 1;
}

long UploadWindow::onCmdPlay(FXObject* sender, FXSelector, void*) {
  FXToggleButton* button = (FXToggleButton*)sender;
  FXuchar state = button->getState();
  if (state == 1) {
    const char* file = "player.ini";
    char filepath[256];
    char* pPos = NULL;
    GetModuleFileNameA(NULL, filepath, sizeof(filepath));
    pPos = strrchr(filepath, '\\');
    *++pPos = '\0';
    strcat(filepath, file);
    RTPPlayerConfig player_config;
    load_player_config(filepath, player_config);

    play_start(player_config);
  }
  else {
    play_stop();
  }
  return 1;
}

long UploadWindow::onCmdLogLevelChanged(FXObject*, FXSelector, void*) {
  int level = loglevel_combo_->getCurrentItem() + 1;
  LogSetLevel(LogLevel(level));
  return 1;
}

long UploadWindow::onCmdProtocolChanged(FXObject*, FXSelector, void*) {
  return 1;
}

long UploadWindow::onCmdReportChanged(FXObject*, FXSelector, void*) {
  return 1;
}

long UploadWindow::onCmdCheckAuto(FXObject* sender, FXSelector, void*) {
  FXCheckButton* checkbox = (FXCheckButton*)sender;
  FXuchar status = checkbox->getCheck();
  if (status == 1) {
    streamgp->hide();
    addrgp->hide();
  }
  else {
    streamgp->show();
    addrgp->show();
  }
  return 1;
}

void UploadWindow::LoadCamera() {
  int count = capture_->GetCameraDevice(camera_devices_, _countof(camera_devices_));
  camera_combo_->clearItems();
  for (int i = 0; i < count; ++i) {
    camera_combo_->appendItem(utf82unicode(camera_devices_[i].szDeviceName).c_str());
  }
  if (count > 0) {
    LoadResolution(0);
  }
}

long UploadWindow::onCmdCameraChanged(FXObject*, FXSelector, void*) {
  FXint cameraindex = camera_combo_->getCurrentItem();
  LoadResolution(cameraindex);
  return 1;
}

void UploadWindow::LoadResolution(int camera_index) {
  if (camera_index < 0 || camera_index > _countof(camera_devices_)) {
    return;
  }

  const char* deviceid = camera_devices_[camera_index].szDeviceID;
  int count = capture_->GetCameraCapability(deviceid, camera_capability_, _countof(camera_capability_));
  res_combo_->clearItems();
  char res_str[64] = { 0 };
  for (int i = 0; i < count; ++i) {
    sprintf(res_str, "%ux%u_%u", camera_capability_[i].width, camera_capability_[i].height, camera_capability_[i].maxFPS);
    res_combo_->appendItem(res_str);
  }
}

void UploadWindow::LoadAudio() {
  int count = capture_->GetAudioRecorderDevice(audio_devices_, _countof(audio_devices_));
  audio_combo_->clearItems();
  for (int i = 0; i < count; ++i) {
    audio_combo_->appendItem(utf82unicode(audio_devices_[i].szDeviceName).c_str());
  }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, char *, int cmdShow) {
  _Module.Init(0, hInstance);

  rtcInit();

  LogSetDir("d:\\rtp_test");
  LogSetLevel(LOG_LEVEL_TRC);

  FXApp application("Upload", "UploadTest");
  int argc1 = 1;
  char *argv1[2] = { "xxx", NULL };
  application.init(argc1, argv1);

  UploadWindow *pWindow = new UploadWindow(&application);
  application.create();

  int ret = application.run();
  rtcUninit();
  return ret;
}
