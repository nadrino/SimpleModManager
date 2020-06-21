//
// Created by Adrien BLANCHET on 20/06/2020.
//

#ifndef SIMPLEMODMANAGER_MAIN_APPLICATION_H
#define SIMPLEMODMANAGER_MAIN_APPLICATION_H

#include <borealis.hpp>

class main_application {

  enum BrowserState{
    GAME_BROWSER = 0,
    MOD_BROWSER
  };

public:
  main_application();
  virtual ~main_application();

  void reset();
  void initialize();
  void initialize_layout();

  BrowserState getCurrentState() const;

  void setCurrentState(BrowserState currentState);

  void start_loop();

private:
  bool _is_initialized_{};

  brls::TabFrame* _rootFrame_;
  brls::List* _browser_list_;

  BrowserState _current_state_;

};


#endif //SIMPLEMODMANAGER_MAIN_APPLICATION_H
