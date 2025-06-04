#include <gtkmm.h>
#include <string>
#include <fstream>
#include <gst/gst.h>
#include <glib.h>
#include <gio/gio.h>

using namespace std;

int a = 0;
GstElement *pipeline = nullptr;

class bonus: public Gtk::Window {
public:
  typedef sigc::signal<void, Glib::ustring> type_signal_value_updated;
  bonus(){
    set_title("БОНУС");
    set_icon_from_file("./j.jpg");
    set_default_size(150,100);
    button1.set_label("БОНУС");
    button1.signal_clicked().connect([this](){
      if ((100>a && a>30)||(a>100)){
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
  Gtk::Button button1;
  type_signal_value_updated m_signal_value_updated;
};

class clicker: public Gtk::Window {
public:
  clicker(){
    set_title("Clicker");
    set_icon_from_file("./j.jpg");
    set_default_size(250,200);
    boox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    add(boox);
    button.set_label(to_string(a));
    button.signal_clicked().connect([this](){
      a+=1;
      ofstream ifile("save", ios::out);
      if (ifile.is_open()){
        ifile << to_string(a);
      }
      button.set_label(to_string(a));
      switch (a){
        case 10:
          set_title("ВАУ");
          break;
        case 25:
          set_title("Больше!!!");
          break;
        case 30:
          clicker::callb();
          break;
        case 100:
          clicker::callb();
          break;
      }
      if ((200>a) && (a>=100)){
        m_timeout_id = Glib::signal_timeout().connect(
          sigc::mem_fun(*this, &clicker::on_timeout),
          1000
        );
      }
      else if (a>200){
        m_timeout_id = Glib::signal_timeout().connect(
          sigc::mem_fun(*this, &clicker::on_timeout),
          100
        );
      }
    });
    label.set_label("GTK кликер!");
    boox.pack_start(label,  Gtk::SHRINK, Gtk::SHRINK, 0);
    boox.pack_end(button, Gtk::EXPAND, Gtk::FILL, 0);

    show_all_children();

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
  };
private:
  Gtk::Button button;
  Gtk::Label label;
  bonus* bon;
  sigc::connection m_timeout_id;
protected:
  Gtk::Box boox;
  void on_bonus_value_updated(const Glib::ustring& new_value_str) {
        button.set_label(new_value_str);
    }

    void on_bonus_window_hidden() {
        if (bon) {
            delete bon;
            bon = nullptr;
        }
    }
  bool on_timeout() {
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
};

int main(int argc, char* argv[]) {
    gst_init(&argc, &argv);
    string b;
    ifstream ifile("save", ios::in);
    if (ifile.is_open()){
      getline(ifile, b);
    }
    a = stoi(b);
    gchar* current_dir_c_str = g_get_current_dir();
    const char* audio_filename = "t.mp3";
    gchar* full_local_path = g_build_filename(current_dir_c_str,
      audio_filename, nullptr);
    GFile* gfile = g_file_new_for_path(full_local_path);
    gchar* file_uri = g_file_get_uri(gfile);
    gchar* launch_string = g_strdup_printf("playbin uri=%s", file_uri);
    pipeline = gst_parse_launch(launch_string, nullptr);
    g_free(current_dir_c_str);
    g_free(full_local_path);
    g_free(file_uri);
    g_free(launch_string);
    g_object_unref(gfile);
    auto app = Gtk::Application::create("org.example.simple");
    clicker windoww;
    return app->run(windoww);
}
