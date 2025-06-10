#include <gtkmm.h>
#include <string>
#include <fstream>
#include <gst/gst.h>
#include <glib.h>
#include <gio/gio.h>
#include <iostream>
#include <ctime>
#include <algorithm>
#include "json.hpp"
#ifdef _WIN32
  #define UNICODE
  #define _UNICODE
  #include <gdk/gdkwin32.h>
  #include <filesystem>
  #include <windows.h>
  #include <shellapi.h>
  #define ID_TRAY_ICON 1001
  #define WM_ACHIEVEMENT_NOTIFY (WM_USER + 100)
#endif

using json = nlohmann::json;
using namespace std;

bool music = true;
int a = 0;
vector<string> achievements = {};
GstElement *pipeline = nullptr;

#ifdef _WIN32
  HWND hwndGlobal = nullptr;
  HICON hAchievementIcon = NULL;
  wstring utf8_to_wstring(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(),
      (int)str.length(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(),
      (int)str.length(), &wstrTo[0], size_needed);
    return wstrTo;
  }
  HICON getAchievementIcon() {
    static HICON hIcon = NULL;
    if (hIcon == NULL) {
        wchar_t exePathBuffer[MAX_PATH]{};
        GetModuleFileNameW(nullptr, exePathBuffer, MAX_PATH);
        std::filesystem::path exeDir = std::filesystem::path(exePathBuffer).
          parent_path();
        std::wstring iconPath = exeDir / L"j.ico";
        if (std::filesystem::exists(iconPath)) {
            hIcon = (HICON)LoadImageW(
                NULL,
                iconPath.c_str(),
                IMAGE_ICON,
                0, 0,
                LR_LOADFROMFILE | LR_DEFAULTSIZE
            );
        }
        if (hIcon == NULL) {
            hIcon = LoadIconW(NULL, IDI_INFORMATION);
        }
    }
    return hIcon;
}
  void cleanupAchievementNotification() {
    NOTIFYICONDATA nid_cleanup = {};
    nid_cleanup.cbSize = sizeof(NOTIFYICONDATA);
    nid_cleanup.uID = ID_TRAY_ICON;
    Shell_NotifyIcon(NIM_DELETE, &nid_cleanup);

    HICON hIcon = getAchievementIcon();
    if (hIcon != NULL) {
        DestroyIcon(hIcon);
    }
  }
  void initTrayIcon(HWND hwnd) {
    hwndGlobal = hwnd;
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hwnd;
    nid.uID = ID_TRAY_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
    nid.uCallbackMessage = WM_ACHIEVEMENT_NOTIFY;
    nid.hIcon = getAchievementIcon();
    wcscpy_s(nid.szTip, L"GtkClicker");
    Shell_NotifyIconW(NIM_ADD, &nid);
    nid.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIconW(NIM_SETVERSION, &nid);
}
#endif

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    GstElement *pipeline = (GstElement*)data;

    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            if (music) {
               gst_element_seek_simple(pipeline, GST_FORMAT_TIME,
                 static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH |
                    GST_SEEK_FLAG_KEY_UNIT), 0);
                    if (music) {
                       gst_element_set_state(pipeline, GST_STATE_PLAYING);
                   } else {
                       gst_element_set_state(pipeline, GST_STATE_NULL);
                   }
           } else {
               gst_element_set_state(pipeline, GST_STATE_NULL);
           }
           break;
        case GST_MESSAGE_ERROR: {
            GError *err;
            gchar *debug;
            gst_message_parse_error(msg, &err, &debug);
            g_printerr("Error: %s\n", err->message);
            g_error_free(err);
            g_free(debug);
            gst_element_set_state(pipeline, GST_STATE_NULL);
            break;
        }
        default:
            break;
    }
    return TRUE;
};

void achget(string achi) {
  #ifdef _WIN32
    if (!hwndGlobal) return;
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwndGlobal;
    nid.uID = ID_TRAY_ICON;
    nid.uFlags = NIF_ICON | NIF_INFO;
    nid.dwInfoFlags = NIIF_USER;
    nid.hIcon = getAchievementIcon();
    std::wstring wtitle = utf8_to_wstring("+Ачивка!");
    std::wstring wmessage = utf8_to_wstring(achi);
    wcsncpy_s(nid.szInfoTitle, wtitle.c_str(), ARRAYSIZE(nid.szInfoTitle));
    wcsncpy_s(nid.szInfo, wmessage.c_str(), ARRAYSIZE(nid.szInfo));
    Shell_NotifyIcon(NIM_MODIFY, &nid);
  #else
    auto app = Gtk::Application::get_default();
    if (!app) return;
    auto noti = Gio::Notification::create("+Ачивка");
    noti->set_body(achi);
    GFile* gfile = g_file_new_for_path("j.png");
    GIcon* gicon = g_file_icon_new(gfile);
    auto icon = Glib::RefPtr<Gio::Icon>(new Gio::Icon(gicon));
    noti->set_icon(icon);
    g_object_unref(gfile);
    app->send_notification(noti);
  #endif
};

void loadgame() {
    ifstream inFile("save.json");
    if (inFile.is_open()) {
        json saveData;
        inFile >> saveData;
        inFile.close();
        a = saveData["points"];
        achievements = saveData["achievements"].get<vector<string>>();
        music = saveData["music"];
    }
};
void savegame() {
    json saveData;
    saveData["points"] = a;
    saveData["achievements"] = achievements;
    saveData["music"] = music;
    ofstream outFile("save.json");
    outFile << saveData;
    outFile.close();
}

class ach: public Gtk::Window{
public:
  ach(){
    set_titlebar(bar);
    bar.set_title("Достижения");
    bar.set_show_close_button(true);
    set_default_size(256,300);
    sw.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    if (find(achievements.begin(), achievements.end(), "1click")
       != achievements.end()){
      c1.set("1click.png");
      r1.add(c1);
    }
    if (find(achievements.begin(), achievements.end(), "500clicks")
       != achievements.end()) {
      c2.set("500clicks.png");
      r2.add(c2);
    }
    if (find(achievements.begin(), achievements.end(), "minigame")
       != achievements.end()) {
      c3.set("minigame.png");
      r3.add(c3);
    }
    if (find(achievements.begin(), achievements.end(), "lost")
       != achievements.end()) {
      c4.set("lost.png");
      r4.add(c4);
    }
    if (find(achievements.begin(), achievements.end(), "winr")
       != achievements.end()) {
      c5.set("winr.png");
      r5.add(c5);
    }
    if (find(achievements.begin(), achievements.end(), "1kclicks")
       != achievements.end()) {
      c6.set("1kclicks.png");
      r6.add(c6);
    }
    if (find(achievements.begin(), achievements.end(), "zero")
       != achievements.end()) {
      c7.set("zero.png");
      r7.add(c7);
    }
    lb.append(r1);
    lb.append(r2);
    lb.append(r3);
    lb.append(r4);
    lb.append(r5);
    lb.append(r6);
    lb.append(r7);
    sw.add(lb);
    add(sw);
    show_all();
  }
private:
  Gtk::HeaderBar bar;
  Gtk::ListBox lb;
  Gtk::ScrolledWindow sw;
  Gtk::Image c1;
  Gtk::ListBoxRow r1;
  Gtk::Image c2;
  Gtk::ListBoxRow r2;
  Gtk::Image c3;
  Gtk::ListBoxRow r3;
  Gtk::Image c4;
  Gtk::ListBoxRow r4;
  Gtk::Image c5;
  Gtk::ListBoxRow r5;
  Gtk::Image c6;
  Gtk::ListBoxRow r6;
  Gtk::Image c7;
  Gtk::ListBoxRow r7;
};

class lot: public Gtk::Window{
public:
  typedef sigc::signal<void, Glib::ustring> type_signal;
  lot(){
    set_titlebar(bar);
    bar.set_title("Лотерея");
    bar.set_show_close_button(true);
    set_icon_from_file("j.png");
    lab.set_text("50%-ничего\n30%-500\n15%-1000\n5%-5000\nЦена 500 кликов.");
    but.set_label("Купить билет!");
    box.set_orientation(Gtk::ORIENTATION_VERTICAL);
    but.signal_clicked().connect([this](){
      y = lotm();
      if (a >= 500){
        a-=500;
        switch (y){
          case 1:
            lab.set_text("Проигрыш");
            break;
          case 2:
            a+=500;
            lab.set_text("+500");
            break;
          case 3:
            a+=1000;
            lab.set_text("+1000");
            break;
          case 4:
            a+=5000;
            lab.set_text("+5000");
            break;
        }
        signal.emit(std::to_string(a));
      }
      else{
        lab.set_text("нужно 500");
      }
      savegame();
    });
    box.pack_start(lab);
    box.pack_start(but);
    add(box);
    show_all();
  }
  type_signal signal_value_updated(){
        return signal;
  }
private:
  int y;
  Gtk::Box box;
  Gtk::Label lab;
  Gtk::Button but;
  Gtk::HeaderBar bar;
  type_signal signal;
  int lotm(){
    int x = 1+ rand() % 100;
    switch (x){
      case 1 ... 50:
        return 1;
        break;
      case 51 ... 80:
        return 2;
        break;
      case 81 ... 95:
        return 3;
        break;
      case 96 ... 100:
        return 4;
        break;
      default:
        return 0;
    }
  }
};

class spin: public Gtk::Window{
public:
  typedef sigc::signal<void, Glib::ustring> type_signal;
  spin(){
    set_titlebar(bar);
    bar.set_title("Спин");
    bar.set_show_close_button(true);
    set_icon_from_file("j.png");
    but.set_label("Крутануть");
    box1.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    box2.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    box3.set_orientation(Gtk::ORIENTATION_VERTICAL);
    but.signal_clicked().connect([this](){
      if (a >=500) {
        x1 = num();
        x2 = num();
        x3 = num();
        pic(x1, a1);
        pic(x2, a2);
        pic(x3, a3);
        if ((x1 == 3)&&(x2 == 3)&& (x3 == 3)){
          a+=50000;
          but.set_label("Победа");
          if (!(find(achievements.begin(), achievements.end(), "winr")
             != achievements.end())){
            achievements.push_back("winr");
            achget("НАША ЗАДАЧА ЛЮБЫМИ СХЕМАМИ\nОБМАНУТЬ ЭТУ РУЛЕТКУ!");
          }
        }
        else{
          a-=500;
          but.set_label("Неудача");
        }
      }
      else{
        but.set_label("Нужно 500");
      }
      signal.emit(std::to_string(a));
      savegame();
    });
    box1.pack_start(a1);
    box1.pack_start(a2);
    box1.pack_start(a3);
    box2.pack_start(but);
    box3.pack_start(box1);
    box3.pack_start(box2);
    add(box3);
    show_all();
  }
  type_signal& signal_value_updated(){
        return signal;
  }
private:
  int x1,x2,x3;
  Gtk::HeaderBar bar;
  Gtk::Image a1;
  Gtk::Image a2;
  Gtk::Image a3;
  Gtk::Button but;
  Gtk::Box box1;
  Gtk::Box box2;
  Gtk::Box box3;
  type_signal signal;
  int num(){
    return 1 + rand() % 5;
  }
  void pic(int& x1, Gtk::Image& a1){
    if (x1 == 1){
      a1.set("1.png");
    }
    else if (x1 == 2){
      a1.set("2.png");
    }
    else if (x1 == 3){
      a1.set("3.png");
    }
    else if (x1 == 4){
      a1.set("4.png");
    }
    else if (x1 == 5){
      a1.set("5.png");
    }
  };
};

class rbg: public Gtk::Window{
public:
  typedef sigc::signal<void, Glib::ustring> type_signal;
  string ruletka(){
    string z;
    int x = 1 + rand() % 100;
    switch (x){
      case 1 ... 24:
        return "black";
        break;
      case 25 ... 49:
        return "red";
        break;
      case 50 ... 51:
        return "green";
        break;
      case 52 ... 75:
        return "red";
        break;
      case 76 ... 100:
        return "black";
        break;
      default:
        return "error";
    };
  }
  rbg(){
    set_titlebar(bar4);
    bar4.set_title("Красное, Чёрное, Зелёное");
    bar4.set_show_close_button(true);
    set_icon_from_file("./j.png");
    image.set("w.png");
    buttonb.set_label("Чёрное");
    buttong.set_label("Зелёное");
    buttonr.set_label("Красное");
    box4.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    box3.set_orientation(Gtk::ORIENTATION_VERTICAL);
    buttonb.signal_clicked().connect([this](){
      if (ruletka() == "black"){
        buttonb.set_label("Победа!");
        a+=500;
        image.set("b.png");
        signal.emit(std::to_string(a));
        savegame();
      }
      else{
        buttonb.set_label("Чёрное");
        a/=2;
        image.set("w.png");
        signal.emit(std::to_string(a));
        savegame();
      }
    });
    buttonr.signal_clicked().connect([this](){
      if (ruletka() == "red"){
        buttonr.set_label("Победа!");
        a+=500;
        image.set("r.png");
        signal.emit(std::to_string(a));
        savegame();
      }
      else{
        buttonr.set_label("Красное");
        a/=2;
        image.set("w.png");
        signal.emit(std::to_string(a));
        savegame();
      }
    });
    buttong.signal_clicked().connect([this](){
      if (ruletka() == "green"){
        buttong.set_label("Победа!");
        a*=2;
        image.set("g.png");
        signal.emit(std::to_string(a));
        if (!(find(achievements.begin(), achievements.end(), "zero")
          != achievements.end())){
          achievements.push_back("zero");
          achget("ВСЁ НА ЗЕРО!");
        }
        savegame();
      }
      else{
        buttong.set_label("Зелёное");
        a/=2;
        image.set("w.png");
        signal.emit(std::to_string(a));
        savegame();
      }
    });
    box4.pack_start(buttonb, Gtk::EXPAND, Gtk::FILL, 0);
    box4.pack_start(buttong, Gtk::EXPAND, Gtk::FILL, 0);
    box4.pack_start(buttonr, Gtk::EXPAND, Gtk::FILL, 0);
    box3.pack_start(image, Gtk::EXPAND, Gtk::FILL, 0);
    box3.pack_start(box4, Gtk::EXPAND, Gtk::FILL, 0);
    add(box3);
    show_all();
  }
  type_signal signal_value_updated() {
        return signal;
    }
private:
  Gtk::Image image;
  Gtk::Box box3;
  Gtk::Box box4;
  Gtk::HeaderBar bar4;
  Gtk::Button buttonb,buttonr,buttong;
  type_signal signal;
};

class bonus: public Gtk::Window {
public:
  typedef sigc::signal<void, Glib::ustring> type_signal_value_updated;
  bonus(){
    set_titlebar(bar2);
    bar2.set_title("БОНУС");
    bar2.set_show_close_button(true);
    set_icon_from_file("./j.png");
    set_default_size(150,100);
    button1.set_label("БОНУС");
    button1.signal_clicked().connect([this](){
      if ((100>a && a>30)||(500>a && a>100)||(1500>a && a>500)||
        (2000>a && a>1500)||(3000>a && a>2000)||(5000>a && a>3000)){
        button1.set_label("Бонус просрочен!");
        button1.set_sensitive(false);
      }
      else{
        switch (a){
          case (30):
            a+=10;
            break;
          case (100):
            a+=20;
            break;
      	  case (500):
      	    a+=100;
      	    break;
          case (1500):
            a+=300;
            break;
          case (2000):
            a+=500;
            break;
          case (3000):
            a+=1000;
            break;
          case (5000):
            a+=1000;
            break;
        }
      m_signal_value_updated.emit(std::to_string(a));
      savegame();
      close();
    }
    });
    add(button1);
    show_all_children();
  };
  type_signal_value_updated signal_value_updated() {
        return m_signal_value_updated;
    }
private:
  Gtk::HeaderBar bar2;
  Gtk::Button button1;
  type_signal_value_updated m_signal_value_updated;
};

class number: public Gtk::Window {
public:
  typedef sigc::signal<void, Glib::ustring> type_signal_value_updated;
  number(){
    set_titlebar(bar1);
    set_default_size(200,200);
    booox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    bar1.set_title("Угадай число");
    bar1.set_show_close_button(true);
    set_icon_from_file("./j.png");
    numg();
    button2.set_label("Ответить!");
    lab.set_text("Угадай моё число");
    cout << x;
    booox.pack_start(lab, Gtk::EXPAND, Gtk::FILL, 0);
    entry.set_placeholder_text("Введите ответ.......");
    booox.pack_start(entry, Gtk::EXPAND, Gtk::FILL, 0);
    booox.pack_start(button2, Gtk::EXPAND, Gtk::FILL, 0);
    button2.signal_clicked().connect([this](){
      Glib::ustring text = entry.get_text();
      if (a < 2){
        close();
      }
      else if (text > to_string(x)){
        lab.set_text("Число больше");
        a/=2;
      }
      else if (text < to_string(x)){
        lab.set_text("Число меньше");
        a/=2;
      }
      else{
        a+=x;
        close();
      }
      m_signal_value_updated.emit(std::to_string(a));
      savegame();
    });
    add(booox);
    show_all();
  };
  type_signal_value_updated signal_value_updated() {
        return m_signal_value_updated;
    }
private:
  int x;
  Glib::ustring text;
  void numg(){
    x = rand();
  };
  Gtk::HeaderBar bar1;
  Gtk::Box booox;
  Gtk::Button button2;
  Gtk::Label lab;
  Gtk::Entry entry;
  type_signal_value_updated m_signal_value_updated;
};

void show_aboutd(Gtk::Window& parent){
  Gtk::AboutDialog about;
  about.set_title("О программе");
  about.set_program_name("GtkClicker");
  about.set_version("Beta-1.0");
  about.set_copyright("copyleft");
  about.set_comments("Это просто кликер :)");
  about.set_authors({"Nickolya02111"});
  about.set_license("GPL-3");
  auto logo = Gdk::Pixbuf::create_from_file("j.png");
  about.set_logo(logo);
  about.set_transient_for(parent);
  about.set_modal(true);
  about.run();
}

class clicker: public Gtk::Window {
public:
  clicker(){
    signal_realize().connect([this]() {
        #ifdef _WIN32
        initWin32Tray();
        #endif
    });
    lb.set_text("Звук(Выкл/Вкл)");
    menui.set_label("Угадайка");
    menui2.set_label("КЧЗ");
    menui3.set_label("Спин");
    menui4.set_label("Лотерея");
    bar.set_title("Clicker");
    bar.set_show_close_button(true);
    set_titlebar(bar);
    set_icon_from_file("./j.png");
    set_default_size(300,250);
    boox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    boox2.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    boox3.set_orientation(Gtk::ORIENTATION_VERTICAL);
    add(boox);
    Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file("."
      "/gtk.png");
    gtkpng.set(pixbuf);
    pb_control();
    pb.set_show_text(true);
    button.set_label(to_string(a));
    button2.set_label(to_string(a));
    buttonach.set_label("Достижения");
    button.add_events(Gdk::BUTTON_PRESS_MASK);
    button2.add_events(Gdk::BUTTON_PRESS_MASK);
    auto imagee = Gtk::make_managed<Gtk::Image>("q.png");
    button4.set_image(*imagee);
    swt.set_active(music);
    button.signal_button_press_event().connect([this](GdkEventButton* event){
      if (event->button == 3) {
        menu.show_all();
        menu.popup_at_pointer(reinterpret_cast<GdkEvent*>(event));
        return true;
      }
      else{
        return false;
      }
    });
    button2.signal_button_press_event().connect([this](GdkEventButton* event){
      if (event->button == 3) {
        menu.show_all();
        menu.popup_at_pointer(reinterpret_cast<GdkEvent*>(event));
        return true;
      }
      else{
        return false;
      }
    });
    swt.property_active().signal_changed().connect([this]() {
      music = swt.get_active();
      if (music) {
          GstStateChangeReturn ret = gst_element_set_state(pipeline,
            GST_STATE_PLAYING);
          g_printerr("Состояние пайплайна изменено на PLAYING. Код: %d\n", ret);
      } else {
          GstStateChangeReturn ret = gst_element_set_state(pipeline,
            GST_STATE_NULL);
          g_printerr("Состояние пайплайна изменено на NULL. Код: %d\n", ret);
      }
      savegame();
    });
    button2.signal_clicked().connect([this](){
      time_t now = time(nullptr);
      tm* local_time = localtime(&now);
      int hour = local_time->tm_hour;
      if (10000>a){
        if (hour >= 6 && hour < 22){
          a+=50;
        }
        else{
          a+=100;
        }
      }
      else if (15000> a){
        if (hour >= 6 && hour < 22){
          a+=75;
        }
        else{
          a+=150;
        }
      }
      else if (15000< a){
        if (hour >= 6 && hour < 22){
          a+=100;
        }
        else{
          a+=200;
        }
      }
      switch (a){
        case 2000:
          bar.set_title("ЙОУ");
          clicker::callb();
          break;
        case 3000:
          bar.set_title("Дайте кофе!!!");
          clicker::callb();
          break;
        case 5000:
          bar.set_title("Кликер");
          clicker::callb();
          break;
      }
      if ((a>=10000) && !menu_has_menui2) {
        menu.append(menui2);
        menu_has_menui2 = true;
        menu.show_all();
      }
      if ((a>=30000) && (gtkpng.get_parent() != &boox)) {
        boox.pack_start(gtkpng, Gtk::SHRINK, Gtk::SHRINK, 0);
        show_all();
        set_title("GTK!");
      }
      pb_control();
      button.set_label(to_string(a));
      button2.set_label(to_string(a));
      savegame();
    });
    menui.signal_activate().connect([this](){
      calln();
    });
    menui2.signal_activate().connect([this](){
      callrbg();
    });
    menui3.signal_activate().connect([this](){
      calls();
    });
    menui4.signal_activate().connect([this](){
      calll();
    });
    button4.signal_clicked().connect([this](){
      show_aboutd(*this);
    });
    buttonach.signal_clicked().connect([this](){
      callach();
    });
    button.signal_clicked().connect([this](){
      time_t now = time(nullptr);
      tm* local_time = localtime(&now);
      int hour = local_time->tm_hour;
      if (hour >= 6 && hour < 22){
        a+=1;
      }
      else{
        a+=2;
      }
      pb_control();
      if (tid.connected()) {
        tid.disconnect();
      }
      button.set_label(to_string(a));
      button2.set_label(to_string(a));
      switch (a){
        case 1 ... 2:
          if (!(find(achievements.begin(), achievements.end(), "1click")
             != achievements.end())){
            achievements.push_back("1click");
            achget("Первый клик!");
          }
          break;
        case 10:
          bar.set_title("ВАУ");
          break;
        case 25:
          bar.set_title("Больше!!!");
          break;
        case 30:
          clicker::callb();
	        bar.set_title("Твой первый бонус!");
          break;
        case 100:
          bar.set_title("С юбилеем!");
          clicker::callb();
          break;
        case 500:
      	  bar.set_title("ЧЕГОО?! 500!");
      	  clicker::callb();
      	  break;
        case 1500:
          bar.set_title("Ещё кнопка)");
          boox.pack_end(button2, Gtk::EXPAND, Gtk::FILL, 0);
          clicker::callb();
          show_all();
        case 2000:
          bar.set_title("ЙОУ");
          clicker::callb();
          break;
        case 3000:
          bar.set_title("Дайте кофе!!!");
          clicker::callb();
          break;
        case 5000:
          bar.set_title("Кликер");
          clicker::callb();
          break;
      }
      if ((200>a && a>=100)){
        tid = Glib::signal_timeout().connect(
          sigc::mem_fun(*this, &clicker::ont),
          3000
        );
      }
      else if (500>a && a>=200){
        tid = Glib::signal_timeout().connect(
          sigc::mem_fun(*this, &clicker::ont),
          2000
        );
      }
      else if (1000>a && a>=500){
       tid = Glib::signal_timeout().connect(
	        sigc::mem_fun(*this, &clicker::ont),
          1000
	     );
       if (!(find(achievements.begin(), achievements.end(), "500clicks")
          != achievements.end())){
         achievements.push_back("500clicks");
         achget("500 кликов!");
       }
      }
      else if (a>=1000){
       tid = Glib::signal_timeout().connect(
	        sigc::mem_fun(*this, &clicker::ont),
          500
	     );
       if (!(find(achievements.begin(), achievements.end(), "1kclicks")
          != achievements.end())){
         achievements.push_back("1kclicks");
         achget("1к кликов!");
       }
      }
      if ((a>=500) && !menu_has_menui3) {
        menu.append(menui3);
        menu_has_menui3 = true;
        menu.show_all();
        if (!(find(achievements.begin(), achievements.end(), "minigame")
          != achievements.end())){
          achievements.push_back("minigame");
          achget("Спин открыт!");
        }
      }
      if ((a>=10000) && !menu_has_menui2) {
        menu.append(menui2);
        menu_has_menui2 = true;
        menu.show_all();
      }
      if ((a>=30000) && (gtkpng.get_parent() != &boox)) {
        boox.pack_start(gtkpng, Gtk::SHRINK, Gtk::SHRINK, 0);
        show_all();
        set_title("GTK!");
      }
      savegame();
    });
    if ((find(achievements.begin(), achievements.end(), "minigame")
      != achievements.end()) && (a<500) && !(find(achievements.begin(),
        achievements.end(), "lost") != achievements.end())) {
        achievements.push_back("lost");
        achget("Почти с нуля");
    }
    bar.pack_start(button4);
    label.set_label("GTK кликер!");
    menu.append(menui4);
    menu.append(menui);
    boox.pack_start(pb, Gtk::EXPAND, Gtk::FILL, 0);
    boox.pack_start(label, Gtk::SHRINK, Gtk::SHRINK, 0);
    boox.pack_end(button, Gtk::EXPAND, Gtk::FILL, 0);
    boox3.pack_end(swt);
    boox3.pack_end(lb);
    boox2.pack_end(boox3);
    boox2.pack_end(buttonach);
    boox.pack_end(boox2);
    if(a>=500){
      menu_has_menui3 = true;
      menu.append(menui3);
    }
    if (a>1500){
      boox.pack_end(button2, Gtk::EXPAND, Gtk::FILL, 0);
      label.set_label("GtkClicker x2");
    };
    if (a>=30000){
      boox.pack_start(gtkpng, Gtk::SHRINK, Gtk::SHRINK, 0);
    }
    if(a>=10000){
      menu_has_menui2 = true;
      menu.append(menui2);
    }
    menu.show_all();
    show_all_children();

    if (music) {
      gst_element_set_state(pipeline, GST_STATE_PLAYING);
    } else {
      gst_element_set_state(pipeline, GST_STATE_NULL);
    }
  };
#ifdef _WIN32
  void initWin32Tray() {
    GtkWidget* widget = GTK_WIDGET(gobj());
    GdkWindow* gdk_window = gtk_widget_get_window(widget);
    HWND hwnd = (HWND)gdk_win32_window_get_handle(gdk_window);
    if (hwnd) {
        initTrayIcon(hwnd);
    }
  }
#endif
private:
  bool menu_has_menui3 = false;
  bool menu_has_menui2 = false;
  GstElement* playbin = nullptr;
  Gtk::Switch swt;
  Gtk::MenuItem menui;
  Gtk::MenuItem menui2;
  Gtk::MenuItem menui3;
  Gtk::MenuItem menui4;
  Gtk::Menu menu;
  Gtk::HeaderBar bar;
  Gtk::Button button;
  Gtk::Button button2;
  Gtk::Button button4;
  Gtk::Button buttonach;
  Gtk::ProgressBar pb;
  Gtk::Label label;
  Gtk::Label lb;
  Gtk::Image gtkpng;
  ach* aa;
  bonus* bon;
  number* num;
  rbg* rbgw;
  spin* s;
  lot* l;
  sigc::connection tid;
protected:
  Gtk::Box boox;
  Gtk::Box boox2;
  Gtk::Box boox3;
  void on_bonus_value_updated(const Glib::ustring& new_value_str) {
        button.set_label(new_value_str);
        button2.set_label(new_value_str);
        pb_control();
    }

    void pb_control(){
      switch (a){
        case 1001 ... 1500:
          pb.set_fraction(a/1500.00);
          pb.set_text("!!!ВТОРАЯ КНОПКА!!!!");
          break;
        case 501 ... 1000:
          pb.set_fraction(a*0.001);
          pb.set_text("Королевский автоклик!");
          break;
        case 201 ... 500:
          pb.set_text("Княжеский автоклик!");
          pb.set_fraction(a/500.00);
          break;
        case 101 ... 200:
          pb.set_text("Боярский автоклик!");
          pb.set_fraction((a-100)*0.01);
          break;
        default:
          pb.set_fraction(a*0.01);
          pb.set_text("");
          break;
      }
    };

    void on_bonus_window_hidden() {
        if (bon) {
            delete bon;
            bon = nullptr;
        }
    }
    void on_number_value_updated(const Glib::ustring& new_value_str) {
          button.set_label(new_value_str);
          button2.set_label(new_value_str);
          pb_control();
      }
    void on_number_window_hidden() {
          if (num) {
              delete num;
              num = nullptr;
          }
      }
      void on_l_value_updated(const Glib::ustring& new_value_str) {
            button.set_label(new_value_str);
            button2.set_label(new_value_str);
            pb_control();
      }
      void on_l_window_hidden() {
            if (l) {
                delete l;
                l = nullptr;
            }
      }
      void on_rbg_value_updated(const Glib::ustring& new_value_str) {
            button.set_label(new_value_str);
            button2.set_label(new_value_str);
            pb_control();
      }
      void on_rbg_window_hidden() {
            if (rbgw) {
                delete rbgw;
                rbgw = nullptr;
            }
      }
      void on_s_value_updated(const Glib::ustring& new_value_str) {
            button.set_label(new_value_str);
            button2.set_label(new_value_str);
            pb_control();
      }
      void on_s_window_hidden() {
            if (s) {
                delete s;
                s = nullptr;
            }
      }
      void on_aa_window_hidden() {
            if (aa) {
                delete aa;
                aa = nullptr;
            }
      }
  bool ont() {
        button.activate();
        return true;
    }
  void callach(){
    aa = new ach();
    aa->signal_hide().connect(
      sigc::mem_fun(*this, &clicker::on_aa_window_hidden)
    );
    aa ->  show();
    aa->set_transient_for(*this);
  }
  void callb(){
    bon = new bonus();
    bon->signal_value_updated().connect(
      sigc::mem_fun(*this, &clicker::on_bonus_value_updated)
   );
    bon->signal_hide().connect(
      sigc::mem_fun(*this, &clicker::on_bonus_window_hidden)
    );
    bon ->  show();
    bon->set_transient_for(*this);
  }
  void calll(){
    l = new lot();
    l->signal_value_updated().connect(
      sigc::mem_fun(*this, &clicker::on_l_value_updated)
   );
    l->signal_hide().connect(
      sigc::mem_fun(*this, &clicker::on_l_window_hidden)
    );
    l ->  show();
    l->set_transient_for(*this);
  }
  void calln(){
    num = new number();
    num->signal_value_updated().connect(
      sigc::mem_fun(*this, &clicker::on_number_value_updated)
   );
    num->signal_hide().connect(
      sigc::mem_fun(*this, &clicker::on_number_window_hidden)
    );
    num ->  show();
    num->set_transient_for(*this);
  }

  void callrbg(){
    rbgw = new rbg();
    rbgw->signal_value_updated().connect(
      sigc::mem_fun(*this, &clicker::on_rbg_value_updated)
   );
    rbgw->signal_hide().connect(
      sigc::mem_fun(*this, &clicker::on_rbg_window_hidden)
    );
    rbgw ->  show();
    rbgw->set_transient_for(*this);
  }
  void calls(){
    s = new spin();
    s->signal_value_updated().connect(
      sigc::mem_fun(*this, &clicker::on_s_value_updated)
   );
    s->signal_hide().connect(
      sigc::mem_fun(*this, &clicker::on_s_window_hidden)
    );
    s ->  show();
    s->set_transient_for(*this);
  }
};

int main(int argc, char* argv[]) {
    srand(time(NULL));
    loadgame();
    gst_init(&argc, &argv);
    gchar* current_dir_c_str = g_get_current_dir();
    gchar* full_local_path = g_build_filename(current_dir_c_str,
      "t.mp3", nullptr);
    GFile* gfile = g_file_new_for_path(full_local_path);
    gchar* file_uri = g_file_get_uri(gfile);
    gchar* launch_string = g_strdup_printf("playbin uri=%s", file_uri);
    pipeline = gst_parse_launch(launch_string, nullptr);
    g_free(current_dir_c_str);
    g_free(full_local_path);
    g_free(file_uri);
    g_free(launch_string);
    g_object_unref(gfile);
    GstBus *bus = gst_element_get_bus(pipeline);
    gst_bus_add_watch(bus, bus_call, pipeline);
    gst_object_unref(bus);
    auto app = Gtk::Application::create("org.example.simple",
      Gio::APPLICATION_HANDLES_OPEN);
    clicker windoww;
    #ifdef _WIN32
      WNDCLASS wc = {};
      wc.lpfnWndProc = DefWindowProc;
      wc.hInstance = GetModuleHandle(nullptr);
      wc.lpszClassName = L"GtkClickerMsgWindow";
      RegisterClass(&wc);
      HWND hMsgWindow = CreateWindow(wc.lpszClassName, nullptr, 0, 0,
        0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
      MSG msg;
      while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
    }
    #endif
    return app->run(windoww);
}
