#ifndef __HEADER_WYC_GUI_EDITOR
#define __HEADER_WYC_GUI_EDITOR

#include "Gwen/Gwen.h"
#include "Gwen/Controls/WindowControl.h"
#include "Gwen/Controls/DockBase.h"
#include "Gwen/Controls/DockedTabControl.h"
#include "Gwen/Controls/Properties.h"
#include "Gwen/Controls/PropertyTree.h"
#include "Gwen/Controls/Property/ColorSelector.h"
#include "Gwen/Controls/Property/Checkbox.h"
#include "Gwen/Controls/MenuStrip.h"
#include "Gwen/Controls/StatusBar.h"
#include "Gwen/Controls/ListBox.h"

#include "wyc/math/transform.h"
#include "wyc/util/string_dict.h"
#include "wyc/render/renderer.h"
#include "wyc/render/texture.h"
#include "wyc/game/input.h"
#include "wyc/obj/object.h"

#include "card_base.h"

namespace wyc
{

class xcard_editor : public xobject
{
	USE_RTTI;
public:
	xcard_editor();
	virtual void on_destroy();
	bool initialize(int width, int height);
	void update();
	void render(wyc::xrenderer *rc);
	bool handle_input(const xinput_buffer &input);
	void open();
	void close();
	void load_packet(const char *file);
	void save_packet(const char *file=0);
	void set_modified(bool b);
	bool is_modified() const;
	xcard* get_preview();
	void get_preview_matrix(xmat4f_t &camera_matrix) const;
	void get_preview_region(xrecti_t &viewport) const;
	void on_login_ok();
	void on_login_fail(int err, const wchar_t *info);
private:
	std::string m_file_path;
	xcard_packet *m_packet;
	xcard_data *m_card;
	xpointer<xtexture> m_avatar;
	std::wstring m_avatar_set;
	std::wstring m_avatar_name;
	// card preview 
	xpointer<xcard> m_preview;
	unsigned m_preview_w, m_preview_h;
	xmat4f_t m_proj_preview;
	xtransform m_camera_preview;
	xspherical<float> m_camera_pos;
	bool m_move_cam, m_ctrl_down;
	bool m_modified;

//--------------------------------------------------
// Gwen data
//--------------------------------------------------
	Gwen::Renderer::Base *m_gwen_rc;
	Gwen::Skin::Base *m_gwen_skin;
	Gwen::Controls::Canvas* m_gwen_canvas;

	Gwen::Controls::MenuStrip *m_main_menu;
	Gwen::Controls::DockBase *m_main_window;
	Gwen::Controls::StatusBar *m_status_bar;

	enum PROPS
	{
		PROP_INDEX=0,
		PROP_NAME,
		PROP_RARITY,
		PROP_TYPE,
		PROP_COST,
		PROP_MAXHP,
		PROP_STRENGTH,
		PROP_DESC,
		PROP_AVATAR_SET,
		PROP_AVATAR_NAME,

		PROP_COUNT,
	};

	struct
	{
		Gwen::Controls::TextBox *version;
		Gwen::Controls::TextBox *search_box;
		Gwen::Controls::ListBox *card_list;
		Gwen::Controls::Property::Base *props[PROP_COUNT];
		Gwen::Controls::MenuItem *menu_login;
		Gwen::Controls::ListBox *avatar_list;
	} m_controls;

	std::string m_msgbox_cmd;

	void _gwen_init(int client_width, int client_height);
	void _gwen_free();
	void _init_main_menu();
	void _init_attribute_panel();
	bool _load_packet();
	void _select_card(unsigned id);
	void _update_card_view(xcard_data *card);
	void _message_box(const std::wstring &title, const std::wstring &msg, const char *cmd=0);
	void _message_box_confirm();
	void _dialog_connection();
	void _load_avatar_set(const std::wstring &avatar_set);

	class xevent_handler : public Gwen::Event::Handler
	{
		xcard_editor *m_editor;
	public:
		xevent_handler(xcard_editor *editor);
		// file menu events
		void on_new_packet (Gwen::Controls::Base *pControl);
		void on_load (Gwen::Controls::Base *pControl);
		void on_save (Gwen::Controls::Base *pControl);
		void on_quit (Gwen::Controls::Base *pControl);
		void on_login_dialog(Gwen::Controls::Base *pControl);
		void on_login(Gwen::Controls::Base *pControl, Gwen::Controls::Base* dialog);
		void on_logout(Gwen::Controls::Base *pControl);
		// attribute panel events
		void on_packet_version(Gwen::Controls::Base *pControl);
		void on_new_card(Gwen::Controls::Base *pControl);
		void on_del_card(Gwen::Controls::Base *pControl);
		void on_search_card(Gwen::Controls::Base *pControl);
		void on_select_card(Gwen::Controls::Base *pControl);
		void on_load_avatar(Gwen::Controls::Base *pControl, Gwen::Controls::Base* text);
		void on_select_avatar(Gwen::Controls::Base* pControl);
		// property modified events
		void on_index_changed(Gwen::Controls::Base* pControl);
		void on_name_changed(Gwen::Controls::Base* pControl);
		void on_rarity_changed(Gwen::Controls::Base* pControl);
		void on_type_changed(Gwen::Controls::Base* pControl);
		void on_cost_changed(Gwen::Controls::Base* pControl);
		void on_maxhp_changed(Gwen::Controls::Base* pControl);
		void on_strength_changed(Gwen::Controls::Base* pControl);
		void on_desc_changed(Gwen::Controls::Base* pControl);
		// misc events
		void on_msgbox_confirm(Gwen::Controls::Base *control, Gwen::Controls::Base* msgbox);
	};
	friend class xevent_handler;
	xevent_handler *m_handler;

	static void _on_avatar_loaded(xcard_editor *self, xtexture *avatar_tex);
};

inline bool xcard_editor::is_modified() const
{
	return m_modified;
}

inline xcard* xcard_editor::get_preview()
{
	return m_preview;
}

inline void xcard_editor::get_preview_matrix(xmat4f_t &camera_matrix) const 
{
	camera_matrix = m_proj_preview * m_camera_preview.world2local();
}

inline void xcard_editor::get_preview_region(xrecti_t &viewport) const 
{
	viewport.set(0,0,m_preview_w,m_preview_h);
}


} // namespace wyc

#endif // __HEADER_WYC_GUI_EDITOR
