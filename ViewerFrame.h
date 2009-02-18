#ifndef __TEXTFRAME_H__
#define __TEXTFRAME_H__

struct Cmiss_command_data;
struct Time_keeper;

class ViewerFrame : public wxFrame
{
public:
  ViewerFrame(Cmiss_command_data* command_data_);
  ViewerFrame(const wxChar* title, int xpos, int ypos, int width, int height);
  ~ViewerFrame();

  wxPanel* getPanel();

private:
  wxTextCtrl *m_pTextCtrl;
  wxMenuBar *m_pMenuBar;
  wxMenu *m_pFileMenu;
  wxMenu *m_pHelpMenu;
  wxPanel* m_pPanel;
  
  Cmiss_command_data* command_data;
  Time_keeper* time_keeper;
  
  bool animationIsOn;

  DECLARE_EVENT_TABLE();

  void Terminate(wxCloseEvent& event);
  void TogglePlay(wxCommandEvent& event);
};

#endif
