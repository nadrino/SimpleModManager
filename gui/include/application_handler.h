//
// Created by Adrien BLANCHET on 16/05/2020.
//

#ifndef SIMPLEMODMANAGER_APPLICATION_HANDLER_H
#define SIMPLEMODMANAGER_APPLICATION_HANDLER_H

#include <render_handler.h>

class application_handler {

public:

  application_handler();
  virtual ~application_handler();

  void reset();
  void initialize();
  void start_main_loop();
  void scan_inputs();

private:

  bool _break_main_loop_;
  bool _is_initialized_;

  render_handler _app_render_handler_;

};


#endif //SIMPLEMODMANAGER_APPLICATION_HANDLER_H
