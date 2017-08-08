#include "../log/log.h"
#include "../log/report.h"
#include "../util/util_common.h"
#include "../network/CHttpFetch.h"
#include "zlib.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <algorithm>
#include <string>
#include <stdint.h>
#include <fstream>

#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#else
#define snprintf _snprintf
#endif

#ifdef WIN32
#include <io.h>
#elif defined(__ANDROID__)
#include <dirent.h>
#include <sys/stat.h>
#include <android/log.h>
#else
#include <glob.h>
#endif

namespace {

#ifdef WIN32
  const char PATH_SLASH = '\\';
#else
  const char PATH_SLASH = '/';
#endif

  const char *log_level2str(int level) {
    switch (level) {
    case live_stream_sdk::LOG_LEVEL_TRC:
      return "TRC";
    case live_stream_sdk::LOG_LEVEL_DBG:
      return "DBG";
    case live_stream_sdk::LOG_LEVEL_INF:
      return "INF";
    case live_stream_sdk::LOG_LEVEL_WRN:
      return "WRN";
    case live_stream_sdk::LOG_LEVEL_ERR:
      return "ERR";
    case live_stream_sdk::LOG_LEVEL_NON:
      return "NON";
    default:
      break;
    }
    return "UNDEFINED";
  }

#ifdef WIN32
  struct tm *localtime_r(time_t* timer, struct tm* tp) {
    struct tm* temp = localtime(timer);
    if (temp == NULL) {
      return NULL;
    }
    *tp = *temp;
    return tp;
  }
#endif

  void DefaultLogCallback(const char *msg) {
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_INFO, "rtc21", "%s", msg);
#else
    printf("%s", msg);
#endif
  }

  char s_deviceid[512] = { 0 };

  void Log2Str(char* buf, int max_len, int level, const char *file, int line_num, const char *format, va_list args) {
    const char* slash = strrchr(file, PATH_SLASH);
    if (slash) {
      file = slash + 1;
    }

    buf[max_len - 1] = 0;
    const char *log_type = log_level2str(level);
#ifdef _WIN32
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    int ret = snprintf(buf, max_len - 1, "%02d:%02d:%02d.%03d %s [%s:%d] ",
      wtm.wHour, wtm.wMinute, wtm.wSecond, wtm.wMilliseconds, log_type, file, line_num);
#else
    struct  timeval    tv;
    struct  timezone   tz;
    struct tm now_time;
    if (gettimeofday(&tv, &tz) != 0) {
      buf[0] = 0;
      return;
    }
    time_t t = tv.tv_sec;
    if (NULL == localtime_r(&t, &now_time)) {
      memset(&now_time, 0, sizeof(now_time));
    }

    int ret = snprintf(buf, max_len - 1, "%02d:%02d:%02d.%03d %s [%s:%d] ",
      now_time.tm_hour, now_time.tm_min, now_time.tm_sec, int(tv.tv_usec / 1000), log_type, file, line_num);
#endif

    if (ret > 0) {
      char temp_formate[2048];
      snprintf(temp_formate, sizeof(temp_formate)-1, "%s deviceid %s\r\n", format, s_deviceid);
      ret += vsnprintf(buf + ret, max_len - ret - 1, temp_formate, args);
    }
  }

}

namespace {
  class LogFile {
  public:
    LogFile(const char* prefix);
    ~LogFile();
    void write(const char* data);
    std::string GetCurrentFileName();

  private:
    std::string m_filename_prefix;
    int m_file_count;
    std::fstream *m_stream;
    pthread_mutex_t _write_mutex;
    std::fstream *m_stream_old;
    unsigned int m_old_time;
    const int nMaxLogFileSize_MB = 8;
  };

  LogFile::LogFile(const char* prefix) {
    m_file_count = 0;
    m_stream = NULL;
    m_stream_old = NULL;
    m_old_time = 0;
    pthread_mutex_init(&_write_mutex, NULL);

    m_filename_prefix = prefix;
    time_t t;
    struct tm now_time;
    t = time(NULL);
    if (localtime_r(&t, &now_time)) {
      char createName[64];
      snprintf(createName, sizeof(createName), "_%04d_%02d_%02d_%02d_%02d_%02d",
        now_time.tm_year + 1900, now_time.tm_mon + 1, now_time.tm_mday, now_time.tm_hour, now_time.tm_min, now_time.tm_sec);
      m_filename_prefix += createName;
    }

    char fullname[1024];
    snprintf(fullname, sizeof(fullname), "%s_%d.log", m_filename_prefix.c_str(), m_file_count);
    m_stream = new std::fstream;
    m_stream->open(fullname, std::ios::out);
  }

  LogFile::~LogFile() {
    if (m_stream) {
      delete m_stream;
    }
    if (m_stream_old) {
      delete m_stream_old;
    }
    pthread_mutex_destroy(&_write_mutex);
  }

  std::string LogFile::GetCurrentFileName() {
    char fullname[1024];
    snprintf(fullname, sizeof(fullname), "%s_%d.log", m_filename_prefix.c_str(), m_file_count);
    return fullname;
  }

  void LogFile::write(const char* data) {
    if (data == NULL || m_stream == NULL || !m_stream->is_open()) {
      return;
    }

    *m_stream << data;

    if (m_stream_old && time(NULL) - m_old_time > 5) {
      // 5 seconds later, close the old file.
      pthread_mutex_lock(&_write_mutex);
      if (m_stream_old && time(NULL) - m_old_time > 5) {
        m_old_time = 0;
        m_stream_old->close();
        delete m_stream_old;
        m_stream_old = NULL;
      }
      pthread_mutex_unlock(&_write_mutex);
    }

    if (nMaxLogFileSize_MB > 0 && m_stream->tellp() > nMaxLogFileSize_MB * 1024 * 1024) {
      pthread_mutex_lock(&_write_mutex);
      if (m_stream_old) {
        // error.
        pthread_mutex_unlock(&_write_mutex);
        return;
      }

      m_file_count++;
      char fullname[1024];
      snprintf(fullname, sizeof(fullname), "%s_%d.log", m_filename_prefix.c_str(), m_file_count);

      std::fstream *new_stream = new std::fstream;
      new_stream->open(fullname, std::ios::out);
      if (!new_stream->is_open()) {
        delete new_stream;
      }
      else {
        m_stream_old = m_stream;
        m_stream = new_stream;
        m_old_time = (unsigned int)time(NULL);
      }
      pthread_mutex_unlock(&_write_mutex);
    }
  }
}

namespace live_stream_sdk {

  const char* GetLiveSDKDeviceID() {
    return s_deviceid;
  }

  void SetLiveSDKDeviceID(const char* deviceid) {
    if (deviceid && deviceid[0]) {
      memset(s_deviceid, 0, sizeof(s_deviceid));
      strcpy(s_deviceid, deviceid);
    }
  }

  typedef void(*RtcLogCallback)(const char *msg);

  // 支持分模块的打印，即不同的模块日志打印到不同的文件
  // 以m_file_network为例，其日志文件前缀为“rtc21_network”，调用printNetwork()即打印到该文件。
  // 如果需要增回模块，以类似m_file_network和printNetwork()的方式添加即可
  class CLog21 {
  public:
    CLog21();
    ~CLog21();

    void init(const char *dir, RtcLogCallback callback);
    void setLevel(LogLevel level);
    LogLevel getLevel();
    const char* getDir();

    void print(int level, const char *file, int line_num, const char *format, va_list args);
    void printWebrtc(int level, const char *file, int line_num, const char *format, va_list args);
    void printNetwork(int level, const char *file, int line_num, const char *format, va_list args);

    // 这是一个定制功能
    // 需求是，我们需要定时将本地log文件发送到server上。
    // 当前正在被使用的文件不应被处理，因此需要此接口返回当前正在被使用的文件。
    void getCurrentUsingFiles(std::vector<std::string> &files);

  private:
    void printImpl(LogFile *logfile, int level, const char *file, int line_num, const char *format, va_list args);
    LogFile *m_file;
    LogFile *m_file_webrtc;
    LogFile *m_file_network;
    RtcLogCallback m_external_print;
    LogLevel m_log_level;
    char m_dir[512];
  };

  CLog21 g_log21;

  CLog21::CLog21() {
    m_file = NULL;
    m_file_webrtc = NULL;
    m_file_network = NULL;
    m_external_print = DefaultLogCallback;
    m_log_level = LOG_LEVEL_INF;
    memset(m_dir, 0, sizeof(m_dir));
  }

  CLog21::~CLog21() {
    if (m_file) {
      delete m_file;
    }
    if (m_file_network) {
      delete m_file_network;
    }
    if (m_file_webrtc) {
      delete m_file_webrtc;
    }
  }

  void CLog21::init(const char *dir, RtcLogCallback callback) {
    if (callback) {
      m_external_print = callback;
    }

    if (dir == NULL || dir[0] == 0) {
      return;
    }
    if (m_file) {
      return;
    }

    std::string path = dir;
    if (path.at(path.length() - 1) == PATH_SLASH) {
      path.resize(path.length() - 1);
    }
    strcpy(m_dir, path.c_str());
    path += PATH_SLASH;

    std::string file = path + "rtc21";
    std::string file_webrtc = path + "rtc21_webrtc";
    std::string file_network = path + "rtc21_network";
    m_file = new LogFile(file.c_str());
    m_file_network = new LogFile(file_network.c_str());
    m_file_webrtc = new LogFile(file_webrtc.c_str());
  }

  void CLog21::printImpl(LogFile *logfile, int level, const char *file, int line_num, const char *format, va_list args) {
    char buf[1024 * 16] = { 0 };
    Log2Str(buf, sizeof(buf), level, file, line_num, format, args);
    if (level >= LOG_LEVEL_INF) {
      m_external_print(buf);
    }
    if (logfile) {
      logfile->write(buf);
    }
  }

  void CLog21::print(int level, const char *file, int line_num, const char *format, va_list args) {
    printImpl(m_file, level, file, line_num, format, args);
  }

  void CLog21::printWebrtc(int level, const char *file, int line_num, const char *format, va_list args) {
    printImpl(m_file_webrtc, level, file, line_num, format, args);
  }

  void CLog21::printNetwork(int level, const char *file, int line_num, const char *format, va_list args) {
    printImpl(m_file_network, level, file, line_num, format, args);
  }

  void CLog21::getCurrentUsingFiles(std::vector<std::string> &files) {
    files.clear();
    if (m_file) {
      files.push_back(m_file->GetCurrentFileName());
    }
    if (m_file_webrtc) {
      files.push_back(m_file_webrtc->GetCurrentFileName());
    }
    if (m_file_network) {
      files.push_back(m_file_network->GetCurrentFileName());
    }
  }

  void CLog21::setLevel(LogLevel level) {
    m_log_level = level;
  }

  LogLevel CLog21::getLevel() {
    return m_log_level;
  }

  const char* CLog21::getDir() {
    return m_dir;
  }

  void LogWebrtcPrintf(int level, const char *file, int line_num, const char *format, va_list args) {
    g_log21.printWebrtc(level, file, line_num, format, args);
  }

  void LogNetworkPrintf(int level, const char *file, int line_num, const char *format, ...) {
    va_list args;
    va_start(args, format);
    g_log21.printNetwork(level, file, line_num, format, args);
    va_end(args);
  }

  void LogPrintf(int level, const char *file, int line_num, const char *format, ...) {
    va_list args;
    va_start(args, format);
    g_log21.print(level, file, line_num, format, args);
    va_end(args);
  }

  LogLevel LogGetLevel() {
    return g_log21.getLevel();
  }

  void LogSetLevel(LogLevel level) {
    g_log21.setLevel(level);
  }

  void LogSetDir(const char *dir, void(*callback)(const char *msg)) {
    g_log21.init(dir, callback);
  }
}

//////////////////////////////////////////////////////////////////////////

namespace {

  const char *getRelativeFilePath(const char* path) {
    if (!path) {
      return "not_found_file_null";
    }
    const char* left_slash = strrchr(path, '/');
    const char* right_slash = strrchr(path, '\\');

    const char* file_name;
    const char* slash = (std::max)(left_slash, right_slash);

    if (slash == NULL) {
      file_name = path;
    }
    else {
      file_name = slash + 1;
    }
    return file_name;
  }

  void getFiles(std::string path, std::vector<std::string>& files) {
#ifdef WIN32
    long   hFile = 0;

    struct _finddata_t fileinfo;
    std::string p;
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
      do {
        if (strstr(fileinfo.name, "rtc21")) {
          files.push_back(p.assign(path).append("\\").append(fileinfo.name));
        }
      } while (_findnext(hFile, &fileinfo) == 0);
      _findclose(hFile);
    }
#elif defined(ANDROID)
    struct dirent* ent = NULL;
    DIR *pDir;
    pDir = opendir(path.c_str());
    if (!pDir)
      return;
    //d_reclen：16表示子目录或以.开头的隐藏文件，24表示普通文本文件,28为二进制文件，还有其他……
    while (NULL != (ent = readdir(pDir))) {
      if (ent->d_type == DT_REG) {
        std::string filename = ent->d_name;
        if (strstr(filename.c_str(), "rtp_upload") || strstr(filename.c_str(), "rtp_download") || strstr(filename.c_str(), "lfrtp")) {
          files.push_back(filename);
        }
      }
    }
    closedir(pDir);
#else
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));
    std::string path1 = path + "/*";
    glob(path1.c_str(), GLOB_TILDE, NULL, &glob_result);
    for (unsigned int i = 0; i < glob_result.gl_pathc; ++i) {
      std::string filename = glob_result.gl_pathv[i];
      if (strstr(filename.c_str(), "rtp_upload") || strstr(filename.c_str(), "rtp_download") || strstr(filename.c_str(), "lfrtp")) {
        files.push_back(filename);
      }
    }
    globfree(&glob_result);

#endif
  }

  int send_file(char* name) {
    int http_code = 0;
    //try
    //{
    //  _http_client.reset(new CHttpClient);
    //  _http_client->SetTimoutMS(3000);
    //  http_code = _http_client->PutFile(_report_url, _report_path, name, deviceid);
    //}
    //catch (BaseException& ex)
    //{
    //  WRN(ex.GetNameAndError().c_str());
    //  return -1;
    //}
    return http_code;
  }

  void compress_send_remind_log_file(const char *log_dir, const char* fliename) {
    char tempName[1024];
    snprintf(tempName, 1024, "%s/%s", log_dir, fliename);
    if (strstr(tempName, ".gz")) {
      int ret = send_file(tempName);
      if (ret == 0) {
        int error = remove(tempName);
        if (error != 0) {
          ERR("remove file %s failed", tempName);
        }
      }
      else {
        ERR("upload log file %s failed", tempName);
      }
    }
    else {
      FILE *fp = fopen(tempName, "rb");
      if (fp) {
        fseek(fp, 0, SEEK_END);
        int len = static_cast<int>(ftell(fp));
        if (len <= 0) {
          return;
        }
        fseek(fp, 0, SEEK_SET);

        static std::vector<char> s_buf;
        if ((int)s_buf.size() < len) {
          s_buf.resize(len);
        }

        fread(&s_buf[0], len, 1, fp);
        fclose(fp);
        int error = remove(tempName);
        if (error != 0) {
          ERR("remove file %s failed", tempName);
        }

        snprintf(tempName, 1024, "%s.gz", tempName);

        gzFile fzip = gzopen(tempName, "wb");
        gzwrite(fzip, &s_buf[0], len);
        gzclose(fzip);

        int ret = send_file(tempName);
        if (ret == 0) {
          int error = remove(tempName);
          if (error != 0)
          {
            ERR("remove file %s failed", tempName);
          }
        }
        else {
          ERR("upload log file %s failed", tempName);
        }
      }
      else {
        WRN("cant open file %s", tempName);
      }
    }
  }

  void send_remind_log_file(const char *log_dir, std::vector<std::string> &current_using_files) {
    if (log_dir[0] == 0) {
      return;
    }

    std::vector<std::string> files;
    getFiles(log_dir, files);
    int size = static_cast<int>(files.size());
    TRC("logpath  %s", log_dir);

    for (int i = 0; i < size; i++) {
      bool is_using = false;
      for (auto it = current_using_files.begin(); it != current_using_files.end(); it++) {
        if (*it == files[i]) {
          is_using = true;
          break;
        }
      }

      if (!is_using) {
        compress_send_remind_log_file(log_dir, getRelativeFilePath(files[i].c_str()));
        TRC("send log file: %s", files[i].c_str());
      }
    }
  }

}

namespace live_stream_sdk {

  CLogReport::CLogReport() {
    m_ev_base = NULL;
    pthread_create(&m_threadid, NULL, WorkerThread, this);
  }

  CLogReport::~CLogReport() {
  }

  void CLogReport::HttpGet(const char *url) {
    CHttpFetch *http = CHttpFetch::Create(true);
    http->Get(url);
  }

  void CLogReport::HttpPost(const char *url, const void *data, int len) {
    CHttpFetch *http = CHttpFetch::Create(true);
    http->Post(url, (const char *)data, len);
  }

  void CLogReport::Quit() {
    if (IsThreadIdValid(&m_threadid)) {
      event_base_loopbreak(m_ev_base);
      pthread_join(m_threadid, NULL);
    }
  }

  void* PTW32_CDECL CLogReport::WorkerThread(void* arg) {
    CLogReport *pThis = (CLogReport*)arg;
    return pThis->WorkerThreadImpl();
  }

  void* CLogReport::WorkerThreadImpl() {
    m_ev_base = event_base_new();

    struct event *ev_timer = event_new(m_ev_base, -1, EV_TIMEOUT, &CLogReport::OnTimer, this);
    struct timeval tv;
    tv.tv_sec = 10 * 60;
    tv.tv_usec = 0;
    event_add(ev_timer, &tv);

    event_base_dispatch(m_ev_base);

    event_del(ev_timer);
    event_base_free(m_ev_base);
    return NULL;
  }

  void CLogReport::OnTimer(evutil_socket_t, short, void *arg) {
    CLogReport *pThis = (CLogReport*)arg;
    pThis->OnTimerImpl();
  }

  void CLogReport::OnTimerImpl() {
    const char *dir = g_log21.getDir();
    std::vector<std::string> current_using_files;
    g_log21.getCurrentUsingFiles(current_using_files);
    send_remind_log_file(dir, current_using_files);
  }

  CLogReport g_report;

}
