//
// Created by Adrien BLANCHET on 16/05/2020.
//

#ifndef SIMPLEMODMANAGER_APPLICATION_HANDLER_H
#define SIMPLEMODMANAGER_APPLICATION_HANDLER_H

#include <render_handler.h>
#include <display_element.h>
#include <rectangle.h>
#include <vector>


class application_handler {

  enum app_state{
    GAME_BROWSER = 0,
    MOD_BROWSER,
    PRESET_MENU
  };

public:

  application_handler();
  virtual ~application_handler();

  void reset();
  void initialize();
  void start_main_loop();
  void scan_inputs();
  void draw_ui();

private:

  bool _break_main_loop_{};
  bool _is_initialized_;

  app_state _current_app_state_;

  // render elements:
  std::vector<draw::display_element*> _elements_list_;

  render_handler _app_render_handler_;


};


#endif //SIMPLEMODMANAGER_APPLICATION_HANDLER_H
