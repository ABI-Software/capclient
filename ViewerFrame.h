#ifndef __TEXTFRAME_H__
#define __TEXTFRAME_H__

class ViewerFrame : public wxFrame
{
public:
  ViewerFrame(const wxChar* title, int xpos, int ypos, int width, int height);
  ~ViewerFrame();

  wxPanel* getPanel();

private:
  wxTextCtrl *m_pTextCtrl;
  wxMenuBar *m_pMenuBar;
  wxMenu *m_pFileMenu;
  wxMenu *m_pHelpMenu;
  wxPanel* m_pPanel;

  DECLARE_EVENT_TABLE();

  void Terminate(wxCloseEvent& event);
};

#endif
