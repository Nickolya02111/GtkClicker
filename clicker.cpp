#include <gtkmm.h>
#include <string>
#include <fstream>
#include <gst/gst.h>
#include <glib.h>
#include <gio/gio.h>
#include <iostream>
#include <ctime>

using namespace std;

int a = 0;
GstElement *pipeline = nullptr;

class spin: public Gtk::Window{
public:
  typedef sigc::signal<void, Glib::ustring> type_signal;
  spin(){
    set_titlebar(bar);
    bar.set_title("Спин");
    bar.set_show_close_button(true);
    set_icon_from_file("j.jpg");
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
          a+=10000;
          but.set_label("Победа");
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
      ofstream ifile("save", ios::out);
      if (ifile.is_open()){
        ifile << to_string(a);
      }
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
    set_icon_from_file("./j.jpg");
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
        ofstream ifile("save", ios::out);
        if (ifile.is_open()){
          ifile << to_string(a);
        }
      }
      else{
        buttonb.set_label("Чёрное");
        a/=2;
        image.set("w.png");
        signal.emit(std::to_string(a));
        ofstream ifile("save", ios::out);
        if (ifile.is_open()){
          ifile << to_string(a);
        }
      }
    });
    buttonr.signal_clicked().connect([this](){
      if (ruletka() == "red"){
        buttonr.set_label("Победа!");
        a+=500;
        image.set("r.png");
        signal.emit(std::to_string(a));
        ofstream ifile("save", ios::out);
        if (ifile.is_open()){
          ifile << to_string(a);
        }
      }
      else{
        buttonr.set_label("Красное");
        a/=2;
        image.set("w.png");
        signal.emit(std::to_string(a));
        ofstream ifile("save", ios::out);
        if (ifile.is_open()){
          ifile << to_string(a);
        }
      }
    });
    buttong.signal_clicked().connect([this](){
      if (ruletka() == "green"){
        buttong.set_label("Победа!");
        a*=2;
        image.set("g.png");
        signal.emit(std::to_string(a));
        ofstream ifile("save", ios::out);
        if (ifile.is_open()){
          ifile << to_string(a);
        }
      }
      else{
        buttong.set_label("Зелёное");
        a/=2;
        image.set("w.png");
        signal.emit(std::to_string(a));
        ofstream ifile("save", ios::out);
        if (ifile.is_open()){
          ifile << to_string(a);
        }
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
    set_icon_from_file("./j.jpg");
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
      ofstream ifile("save", ios::out);
      if (ifile.is_open()){
        ifile << to_string(a);
      }
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
    set_icon_from_file("./j.jpg");
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
      ofstream ifile("save", ios::out);
      if (ifile.is_open()){
        ifile << to_string(a);
      }
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
    srand(time(NULL));
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
  about.set_version("Alpha3");
  about.set_copyright("copyleft");
  about.set_comments("Это просто кликер :)");
  about.set_authors({"Nickolya02111"});
  about.set_license("GPL-3");
  auto logo = Gdk::Pixbuf::create_from_file("j.jpg");
  about.set_logo(logo);
  about.set_transient_for(parent);
  about.set_modal(true);
  about.run();
}

class clicker: public Gtk::Window {
public:
  clicker(){
    Gtk::Box* bsb = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 5));
    menui.set_label("Угадайка");
    menui2.set_label("КЧЗ");
    menui3.set_label("Спин");
    bar.set_title("Clicker");
    bar.set_show_close_button(true);
    set_titlebar(bar);
    set_icon_from_file("./j.jpg");
    set_default_size(300,250);
    boox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    add(boox);
    Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file("."
      "/gtk.png");
    gtkpng.set(pixbuf);
    pb_control();
    pb.set_show_text(true);
    button.set_label(to_string(a));
    button2.set_label(to_string(a));
    button.add_events(Gdk::BUTTON_PRESS_MASK);
    button2.add_events(Gdk::BUTTON_PRESS_MASK);
    auto imagee = Gtk::make_managed<Gtk::Image>("q.png");
    button4.set_image(*imagee);
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
      gchar* current_dir_c_str = g_get_current_dir();
      gchar* full_local_path = g_build_filename(current_dir_c_str,
        "click.wav", nullptr);
      GFile* gfile = g_file_new_for_path(full_local_path);
      gchar* file_uri = g_file_get_uri(gfile);
      g_free(current_dir_c_str);
      g_free(full_local_path);
      g_object_unref(gfile);
      gst_element_set_state(playbin, GST_STATE_NULL);
      gst_object_unref(playbin);
      playbin = gst_element_factory_make("playbin", "pb");
      g_object_set(G_OBJECT(playbin), "uri", file_uri, NULL);
      gst_element_set_state(playbin, GST_STATE_PLAYING);
      pb_control();
      ofstream ifile("save", ios::out);
      if (ifile.is_open()){
        ifile << to_string(a);
      }
      button.set_label(to_string(a));
      button2.set_label(to_string(a));
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
    button4.signal_clicked().connect([this](){
      show_aboutd(*this);
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
      ofstream ifile("save", ios::out);
      if (ifile.is_open()){
        ifile << to_string(a);
      }
      if (tid.connected()) {
        tid.disconnect();
      }
      gchar* current_dir_c_str = g_get_current_dir();
      gchar* full_local_path = g_build_filename(current_dir_c_str,
        "click.wav", nullptr);
      GFile* gfile = g_file_new_for_path(full_local_path);
      gchar* file_uri = g_file_get_uri(gfile);
      g_free(current_dir_c_str);
      g_free(full_local_path);
      g_object_unref(gfile);
      gst_element_set_state(playbin, GST_STATE_NULL);
      gst_object_unref(playbin);
      playbin = gst_element_factory_make("playbin", "pb");
      g_object_set(G_OBJECT(playbin), "uri", file_uri, NULL);
      gst_element_set_state(playbin, GST_STATE_PLAYING);
      button.set_label(to_string(a));
      button2.set_label(to_string(a));
      switch (a){
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
      }
      else if (a>=1000){
       tid = Glib::signal_timeout().connect(
	        sigc::mem_fun(*this, &clicker::ont),
          50
	     );
      }
      if ((a>=500) && !menu_has_menui3) {
        menu.append(menui3);
        menu_has_menui3 = true;
        menu.show_all();
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
    });
    bar.pack_start(button4);
    label.set_label("GTK кликер!");
    menu.append(menui);
    boox.pack_start(pb, Gtk::EXPAND, Gtk::FILL, 0);
    boox.pack_start(label, Gtk::SHRINK, Gtk::SHRINK, 0);
    boox.pack_end(button, Gtk::EXPAND, Gtk::FILL, 0);
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

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
  };
private:
  bool menu_has_menui3 = false;
  bool menu_has_menui2 = false;
  GstElement* playbin = nullptr;
  Gtk::MenuItem menui;
  Gtk::MenuItem menui2;
  Gtk::MenuItem menui3;
  Gtk::Menu menu;
  Gtk::HeaderBar bar;
  Gtk::Button button;
  Gtk::Button button2;
  Gtk::Button button4;
  Gtk::ProgressBar pb;
  Gtk::Label label;
  Gtk::Image gtkpng;
  bonus* bon;
  number* num;
  rbg* rbgw;
  spin* s;
  sigc::connection tid;
protected:
  Gtk::Box boox;
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
  bool ont() {
        button.activate();
        return true;
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

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    GstElement *pipeline = (GstElement*)data;

    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            gst_element_seek_simple(pipeline, GST_FORMAT_TIME,
              static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH |
                 GST_SEEK_FLAG_KEY_UNIT), 0);
            gst_element_set_state(pipeline, GST_STATE_PLAYING);
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

int main(int argc, char* argv[]) {
    srand(time(NULL));
    gst_init(&argc, &argv);
    string b;
    ifstream ifile("save", ios::in);
    if (ifile.is_open()){
      getline(ifile, b);
    }
    a = stoi(b);
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
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    auto app = Gtk::Application::create("org.example.simple");
    clicker windoww;
    return app->run(windoww);
}
