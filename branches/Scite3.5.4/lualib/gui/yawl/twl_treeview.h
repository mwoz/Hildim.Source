#ifndef __TWL_TREEVIEW_H
#define __TWL_TREEVIEW_H
class EXPORT TTreeView: public TNotifyWin {
	SelectionHandler       m_on_select;
	NotifyEventHandler m_on_selection_changing;
	TEventWindow* m_form;
    bool m_has_images;
public:

  void on_select(SelectionHandler handler)
  { m_on_select = handler; }

  void on_selection_changing(NEH handler)
  { m_on_selection_changing = handler; }


   TTreeView(TEventWindow* form, bool has_lines = true, bool editable = false);
   Handle add(Handle parent, const wchar_t* caption, int idx1=0, int idx2=-1, bool has_children=false, void* data=0);
   void* get_data(Handle pn);
   void select(Handle p);
   void set_image_list(TImageList* il, bool normal = true);
   // override
   int handle_notify(void *p);
   Handle GetSelection();
};

#endif
