#include "fscorepch.h"
#include "gui_editor.h"

#include "Gwen/Skins/TexturedBase.h"

#include "wyc/util/strutil.h"
#include "wyc/game/glb_game.h"

#include "gwen_renderer.h"
#include "gwen_pwd.h"

extern void login(const std::string &usr, const std::string &pwd);
extern void logout();
extern int get_login_state();

namespace wyc
{

REG_RTTI(xcard_editor, xobject)

xcard_editor::xcard_editor()
{
	m_file_path = "card_packet.json";
	get_resource_path(m_file_path);
	m_packet=0;
	m_card=0;
	m_preview = wycnew xcard();
	m_preview_w = m_preview_h = 0;
	m_move_cam=false;
	m_ctrl_down=false;
	m_modified=false;

	m_gwen_rc=0;
	m_gwen_skin=0;
	m_gwen_canvas=0;
	m_handler=0;
	m_main_window=0;
	m_main_menu=0;
	m_status_bar=0;
	memset(&m_controls,0,sizeof(m_controls));
}

void xcard_editor::on_destroy()
{
	if(m_packet)
	{
		delete m_packet;
		m_packet=0;
		m_card=0;
	}
	m_preview=0;
	m_avatar=0;
	_gwen_free();
}

bool xcard_editor::initialize(int width, int height)
{
	if(m_gwen_canvas)
		return true;
	// setup control panel
	_gwen_init(width,height);
	_init_main_menu();
	_init_attribute_panel();
	// setup preview panel
	m_preview_w=width-200;
	m_preview_h=height-m_main_menu->GetBounds().h-m_status_bar->GetBounds().h;
	set_perspective(m_proj_preview,45,float(m_preview_w)/m_preview_h,1,1000);
	m_camera_pos.r=6.0f;
	m_camera_pos.longitude=DEG_TO_RAD(-90);
	m_camera_pos.latitude=DEG_TO_RAD(90);
	m_camera_preview.set_position(0,0,m_camera_pos.r);
	m_camera_preview.set_forward(xvec3f_t(0,0,m_camera_pos.r),xvec3f_t(0,1,0));
	// load data
	_load_packet();
	return true;
}

void xcard_editor::update() 
{
	m_camera_preview.update();
}

void xcard_editor::render(wyc::xrenderer *rc)
{
	// Gwen gui canvas
	rc->use_shader(0);
	const Gwen::Rect &client_rect = m_gwen_canvas->GetBounds();
	glViewport(0,0,client_rect.w,client_rect.h);
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0, client_rect.w, client_rect.h, 0, -1.0, 1.0);
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	m_gwen_canvas->RenderCanvas();
}

bool xcard_editor::handle_input (const xinput_buffer &input)
{
	// handle mouse input
	bool repos=false;
	if(input.offz) {
	//	wyc_print("wheel:%d",input.offz);
		m_camera_pos.r=std::min(8.0f, std::max(m_camera_pos.r-input.offz*0.002f,4.0f) );
		repos=true;
	}
	if(m_move_cam) {
	//	wyc_print("cursor:(%d,%d) (%d,%d)",input.offx,input.offy,input.x,input.y);
		if(input.offx!=0 || input.offy!=0) {
			m_camera_pos.longitude-=input.offx*0.005f;
			m_camera_pos.latitude-=input.offy*0.005f;
			if(m_camera_pos.longitude<0)
				m_camera_pos.longitude+=float(XMATH_2PI);
			else if(m_camera_pos.longitude>=XMATH_2PI)
				m_camera_pos.longitude-=float(XMATH_2PI);
			if(m_camera_pos.latitude<0.14f)
				m_camera_pos.latitude=0.14f;
			else if(m_camera_pos.latitude>3.0f)
				m_camera_pos.latitude=3.0f;
			repos=true;
		}
	}
	if(repos) {
		wyc::xvec3f_t npos;
		wyc::to_cartesian(m_camera_pos,npos);
		std::swap(npos.y,npos.z);
		npos.z=-npos.z;
		m_camera_preview.set_position(npos);
		m_camera_preview.set_forward(npos,xvec3f_t(0,1,0));
	}
	for(xmouseque::const_iterator iter=input.mouseque.begin(), 
		end=input.mouseque.end(); iter!=end; ++iter) {
		if(iter->msg==wyc::EV_RB_DOWN) {
			m_move_cam=true;
		}
		else if(iter->msg==wyc::EV_RB_UP) {
			m_move_cam=false;
		}
	}
	for(xkeyque::const_iterator iter = input.keyque.begin(),
		end = input.keyque.end(); iter != end; ++iter)
	{
		if(iter->key == VK_CONTROL)
			m_ctrl_down = iter->msg == EV_KEY_DOWN;
	}

	if(input.move)
	{
		m_gwen_canvas->InputMouseMoved(input.x, input.y, input.offx, input.offy);
	}
	if(input.offz)
		m_gwen_canvas->InputMouseWheel(input.offz);

	for(xmouseque::const_iterator iter=input.mouseque.begin(), 
		end=input.mouseque.end(); iter!=end; ++iter) {
		switch(iter->msg)
		{
		case EV_LB_DOWN:
			m_gwen_canvas->InputMouseButton(0,true);
			break;
		case EV_LB_UP:
			m_gwen_canvas->InputMouseButton(0,false);
			break;
		case EV_RB_DOWN:
			m_gwen_canvas->InputMouseButton(1,true);
			break;
		case EV_RB_UP:
			m_gwen_canvas->InputMouseButton(1,false);
			break;
		case EV_MB_DOWN:
			m_gwen_canvas->InputMouseButton(2,true);
			break;
		case EV_MB_UP:
			m_gwen_canvas->InputMouseButton(2,false);
			break;
		}
	}
	bool key_is_handled = false;
	for(xkeyque::const_iterator iter = input.keyque.begin(),
		end = input.keyque.end(); iter != end; ++iter)
	{
		int iKey = -1;

		if ( iter->key == VK_SHIFT ) iKey = Gwen::Key::Shift;
		else if ( iter->key == VK_RETURN ) iKey = Gwen::Key::Return;
		else if ( iter->key == VK_BACK ) iKey = Gwen::Key::Backspace;
		else if ( iter->key == VK_DELETE ) iKey = Gwen::Key::Delete;
		else if ( iter->key == VK_LEFT ) iKey = Gwen::Key::Left;
		else if ( iter->key == VK_RIGHT ) iKey = Gwen::Key::Right;
		else if ( iter->key == VK_TAB ) iKey = Gwen::Key::Tab;
		else if ( iter->key == VK_SPACE ) iKey = Gwen::Key::Space;
		else if ( iter->key == VK_HOME ) iKey = Gwen::Key::Home;
		else if ( iter->key == VK_END ) iKey = Gwen::Key::End;
		else if ( iter->key == VK_CONTROL ) iKey = Gwen::Key::Control;
		else if ( iter->key == VK_SPACE ) iKey = Gwen::Key::Space;
		else if ( iter->key == VK_UP ) iKey = Gwen::Key::Up;
		else if ( iter->key == VK_DOWN ) iKey = Gwen::Key::Down;

		if(iKey!=-1)
		{
			key_is_handled |= m_gwen_canvas->InputKey(iKey, iter->msg==EV_KEY_DOWN);
		}
		else if(iter->msg == EV_KEY_DOWN && m_ctrl_down && iter->key>='A' && iter->key<='Z')
		{
			key_is_handled |= m_gwen_canvas->InputCharacter( Gwen::UnicodeChar(iter->key) );
		}

	}
	if(!input.chars.empty())
	{
		for(size_t i=0; i<input.chars.size(); ++i)
		{
			key_is_handled |= m_gwen_canvas->InputCharacter( Gwen::UnicodeChar(input.chars[i]) );
		}
	}
	return key_is_handled;
}

void xcard_editor::open()
{
}

void xcard_editor::close()
{
}

void xcard_editor::_gwen_init(int client_width, int client_height)
{
	std::string path;
	wyc::xressvr *svr = get_resource_server();

	// initialize resource
	path="font/wqy10.font";
	get_resource_path(path);
	xfont *font_chn = (xfont*)svr->request(xfont::get_class()->id,path.c_str());
	assert(font_chn->is_complete());

	xgwen_renderer *rc = new xgwen_renderer();
	rc->set_font(font_chn);
	Gwen::Skin::TexturedBase *skin = new Gwen::Skin::TexturedBase();
	skin->SetRender(rc);
	path = "gui/DefaultSkin.png";
	get_resource_path(path);
	skin->Init(path.c_str());

	Gwen::Controls::Canvas* canvas = new Gwen::Controls::Canvas( skin );
	canvas->SetSize( client_width, client_height );
	canvas->SetDrawBackground( false );
	canvas->SetBackgroundColor( Gwen::Color( 150, 170, 170, 255 ) );

	m_gwen_rc = rc;
	m_gwen_skin = skin;
	m_gwen_canvas = canvas;
	m_handler = new xevent_handler(this);
	m_main_window = new Gwen::Controls::DockBase(canvas);
	m_main_window->Dock(Gwen::Pos::Fill);
}

void xcard_editor::_gwen_free()
{
	if(!m_gwen_canvas) 
		return;
	delete m_gwen_canvas;
	m_gwen_canvas = 0;
	delete m_gwen_skin;
	m_gwen_skin = 0;
	delete m_gwen_rc;
	m_gwen_rc = 0;
	delete m_handler;
	m_handler=0;
	m_main_window=0;
	m_main_menu=0;
	m_status_bar=0;
	memset(&m_controls,0,sizeof(m_controls));
}

void xcard_editor::_init_main_menu ()
{
	// status bar
	if(!m_status_bar) {
		m_status_bar = new Gwen::Controls::StatusBar(m_gwen_canvas);
		m_status_bar->SetText(L"",false);
	}
	if(m_main_menu)
		return;
	m_main_menu = new Gwen::Controls::MenuStrip(m_main_window);
	m_main_menu->Dock(Gwen::Pos::Top);
	Gwen::Controls::MenuItem *root_item = m_main_menu->AddItem(L"文件");
	Gwen::Controls::Menu *sub_menu = root_item->GetMenu();
//	sub_menu->AddItem(L"新建",L"",L"Ctrl+N")->SetAction(m_handler,&xevent_handler::on_new_packet);
//	sub_menu->AddItem(L"加载")->SetAction(m_handler,&xevent_handler::on_load);
	sub_menu->AddItem(L"保存",L"",L"Ctrl+S")->SetAction(m_handler,&xevent_handler::on_save);
//	sub_menu->AddItem(L"详细信息")->SetAction(m_handler,&xevent_handler::on_show_packet_info);
	m_controls.menu_login = sub_menu->AddItem(L"登录...")->SetAction(m_handler,&xevent_handler::on_login_dialog);
	sub_menu->AddDivider();
	sub_menu->AddItem(L"关闭",L"",L"Alt+F4")->SetAction(m_handler,&xevent_handler::on_quit);

	root_item = m_main_menu->AddItem(L"帮助");
	sub_menu = root_item->GetMenu();
	sub_menu->AddItem(L"关于",L"",L"F1");

}

void xcard_editor::_init_attribute_panel()
{
	Gwen::Controls::DockBase *panel = m_main_window->GetRight();
	panel->SetWidth( 200 );
	
	Gwen::Controls::TabControl *tabs = panel->GetTabControl();
	Gwen::Controls::Base *page = new Gwen::Controls::Base(m_main_window);
	tabs->AddPage(L"卡牌设计", page);
	
	// 全局属性
	Gwen::Controls::Base *item;
	Gwen::Controls::Label *label; 
	Gwen::Controls::TextBox *text;
	Gwen::Controls::Button *button;
	item = new Gwen::Controls::Base(page);
	label = new Gwen::Controls::Label(item);
	text = new Gwen::Controls::TextBox(item);
	label->SetText(L"版本",false);
	label->SetAlignment(Gwen::Pos::CenterV|Gwen::Pos::Left);
	label->SizeToContents();
	label->Dock(Gwen::Pos::Left);
	text->SetMargin(Gwen::Margin(10));
	text->Dock(Gwen::Pos::Fill);
	text->onTextChanged.Add(m_handler,&xevent_handler::on_packet_version);
	item->SizeToChildren();
	item->Dock(Gwen::Pos::Top);	
	m_controls.version=text;

	// 卡牌列表和搜索框
	label = new Gwen::Controls::Label(page);
	label->SetText(L"选择要编辑的卡牌",false);
	label->SizeToContents();
	label->SetMargin(Gwen::Margin(0,6));
	label->Dock(Gwen::Pos::Top);
	text = new Gwen::Controls::TextBox(page);
	text->SetMargin(Gwen::Margin(0,4));
	text->Dock(Gwen::Pos::Top);
	text->onTextChanged.Add(m_handler,&xevent_handler::on_search_card);
	Gwen::Controls::ListBox *list = new Gwen::Controls::ListBox(page);
	list->SetColumnCount(2);
	list->SetMargin(Gwen::Margin(0,4));
	list->SetHeight(120);
	list->Dock(Gwen::Pos::Top);
	list->onRowSelected.Add(m_handler,&xevent_handler::on_select_card);
	m_controls.search_box=text;
	m_controls.card_list=list;

	item = new Gwen::Controls::Base(page);
	button = new Gwen::Controls::Button(item);
	button->SetText(L"删除",false);
	button->SetWidth(60);
	button->Dock(Gwen::Pos::Right);
	button->onPress.Add(m_handler,&xevent_handler::on_del_card);
	button = new Gwen::Controls::Button(item);
	button->SetText(L"添加",false);
	button->SetWidth(60);
	button->SetMargin(Gwen::Margin(0,0,6));
	button->Dock(Gwen::Pos::Right);
	button->onPress.Add(m_handler,&xevent_handler::on_new_card);
	item->SizeToChildren();
	item->SetMargin(Gwen::Margin(0,6));	
	item->Dock(Gwen::Pos::Top);
	
	// 属性树
	Gwen::Controls::PropertyTree *tree = new Gwen::Controls::PropertyTree(page);
	Gwen::Controls::Properties *props;
	Gwen::Controls::PropertyRow *row;
	tree->SetMargin(Gwen::Margin(0,6));
	props = tree->Add(L"卡牌属性");
	row = props->Add(L"编号");
	row->onChange.Add(m_handler, &xevent_handler::on_index_changed);
	m_controls.props[PROP_INDEX] = row->GetProperty();
	row = props->Add(L"名字");
	row->onChange.Add(m_handler, &xevent_handler::on_name_changed);
	m_controls.props[PROP_NAME] = row->GetProperty();
	row = props->Add(L"稀有度");
	row->onChange.Add(m_handler, &xevent_handler::on_rarity_changed);
	m_controls.props[PROP_RARITY] = row->GetProperty();
	row = props->Add(L"类型");
	row->onChange.Add(m_handler, &xevent_handler::on_type_changed);
	m_controls.props[PROP_TYPE] = row->GetProperty();
	row = props->Add(L"费用");
	row->onChange.Add(m_handler, &xevent_handler::on_cost_changed);
	m_controls.props[PROP_COST] = row->GetProperty();
	row = props->Add(L"力量");
	row->onChange.Add(m_handler, &xevent_handler::on_strength_changed);
	m_controls.props[PROP_STRENGTH] = row->GetProperty();
	row = props->Add(L"生命");
	row->onChange.Add(m_handler, &xevent_handler::on_maxhp_changed);
	m_controls.props[PROP_MAXHP] = row->GetProperty();
	row = props->Add(L"描述");
	row->onChange.Add(m_handler, &xevent_handler::on_desc_changed);
	m_controls.props[PROP_DESC] = row->GetProperty();
	row = props->Add(L"图像");
	m_controls.props[PROP_AVATAR_NAME] = row->GetProperty();
	row = props->Add(L"图像集");
	m_controls.props[PROP_AVATAR_SET] = row->GetProperty();


	tree->ExpandAll();
	tree->SetHeight(200);
	tree->Dock(Gwen::Pos::Top);

	// avatar面板
	label = new Gwen::Controls::Label(page);
	label->SetText(L"图像文件路径",false);
	label->SizeToContents();
	label->SetMargin(Gwen::Margin(0,6));
	label->Dock(Gwen::Pos::Top);

	item = new Gwen::Controls::Base(page);
	button = new Gwen::Controls::Button(item);
	button->SetText(L"加载",false);
	button->SetWidth(60);
	button->Dock(Gwen::Pos::Right);
	text = new Gwen::Controls::TextBox(item);
	text->SetMargin(Gwen::Margin(0,0,6));
	text->Dock(Gwen::Pos::Fill);
	button->onPress.Add(m_handler,&xevent_handler::on_load_avatar,text);
	item->SizeToChildren();
	item->SetMargin(Gwen::Margin(0,6));	
	item->Dock(Gwen::Pos::Top);

	list = new Gwen::Controls::ListBox(page);
	list->SetMargin(Gwen::Margin(0,4));
	list->SetHeight(120);
	list->Dock(Gwen::Pos::Top);
	list->onRowSelected.Add(m_handler,&xevent_handler::on_select_avatar);
	m_controls.avatar_list=list;
}

void xcard_editor::_message_box(const std::wstring &title, const std::wstring &msg, const char *cmd)
{
	Gwen::Controls::WindowControl* window = new Gwen::Controls::WindowControl(m_gwen_canvas);
	Gwen::Controls::Label *label = new Gwen::Controls::Label(window);
	label->SetText(msg,false);
	label->SetAlignment(Gwen::Pos::CenterH|Gwen::Pos::CenterV);
	label->SizeToContents();
	const Gwen::Rect &bound = label->GetBounds();
	window->SetSize(std::max(bound.w+40,200), bound.h+100);
	label->SetMargin(Gwen::Margin(0,20,0,20));
	label->Dock(Gwen::Pos::Top);

	Gwen::Controls::Button *btn;
	if(cmd) {
		btn=new Gwen::Controls::Button(window);
		btn->SetText(L"取消");
		btn->SetWidth(60);
		btn->Dock(Gwen::Pos::Right);
		btn->onPress.Add(window,&Gwen::Controls::WindowControl::CloseButtonPressed);
		btn=new Gwen::Controls::Button(window);
		btn->SetText(L"确定");
		btn->SetWidth(60);
		btn->SetMargin(Gwen::Margin(0,0,6));
		btn->Dock(Gwen::Pos::Right);
		btn->onPress.Add(m_handler,&xevent_handler::on_msgbox_confirm,window);
		m_msgbox_cmd=cmd;
	}
	else {
		btn=new Gwen::Controls::Button(window);
		btn->SetText(L"确定");
		btn->SetWidth(60);
		btn->Dock(Gwen::Pos::Right);
		btn->onPress.Add(window,&Gwen::Controls::WindowControl::CloseButtonPressed);
		m_msgbox_cmd.clear();
	}

	window->SetTitle(title);
	window->Position( Gwen::Pos::Center );
	// TODO: 存在modal窗口的时候,删除canvas会导致崩溃,等待新版本的Gwen修正此问题
	window->MakeModal( true );
	window->SetDeleteOnClose( true );
}

void xcard_editor::_dialog_connection()
{
	Gwen::Controls::WindowControl* window = new Gwen::Controls::WindowControl(m_gwen_canvas);
	Gwen::Controls::Base *item;
	Gwen::Controls::Label *label; 
	Gwen::Controls::TextBox *text;

	item = new Gwen::Controls::Base(window);
	label = new Gwen::Controls::Label(item);
	text = new Gwen::Controls::TextBox(item);
	label->SetText(L"帐号",false);
	label->SetAlignment(Gwen::Pos::CenterV|Gwen::Pos::Left);
	label->SizeToContents();
	label->Dock(Gwen::Pos::Left);
	text->SetName("usr");
	text->SetMargin(Gwen::Margin(10));
	text->Dock(Gwen::Pos::Fill);
	text->Focus();
	item->SizeToChildren();
	item->SetMargin(Gwen::Margin(0,8));
	item->Dock(Gwen::Pos::Top);	

	item = new Gwen::Controls::Base(window);
	label = new Gwen::Controls::Label(item);
	text = new Gwen::Controls::SecureText(item);
	label->SetText(L"密码",false);
	label->SetAlignment(Gwen::Pos::CenterV|Gwen::Pos::Left);
	label->SizeToContents();
	label->Dock(Gwen::Pos::Left);
	text->SetName("pwd");
	text->SetMargin(Gwen::Margin(10));
	text->Dock(Gwen::Pos::Fill);
	text->onReturnPressed.Add(m_handler,&xevent_handler::on_login,window);
	item->SizeToChildren();
	item->SetMargin(Gwen::Margin(0,8,0,8));
	item->Dock(Gwen::Pos::Top);	

	Gwen::Controls::Button *btn;
	btn=new Gwen::Controls::Button(window);
	btn->SetText(L"取消");
	btn->SetWidth(60);
	btn->Dock(Gwen::Pos::Right);
	btn->onPress.Add(window,&Gwen::Controls::WindowControl::CloseButtonPressed);
	btn=new Gwen::Controls::Button(window);
	btn->SetText(L"确定");
	btn->SetWidth(60);
	btn->SetMargin(Gwen::Margin(0,0,6));
	btn->Dock(Gwen::Pos::Right);
	btn->onPress.Add(m_handler,&xevent_handler::on_login,window);

	window->SetTitle(L"登录");
	window->SetSize(200,120);
	window->Position( Gwen::Pos::Center );
	// TODO: 存在modal窗口的时候,删除canvas会导致崩溃,等待新版本的Gwen修正此问题
	window->MakeModal( true );
	window->SetDeleteOnClose( true );
}

void xcard_editor::set_modified(bool b)
{
	if(m_modified==b)
		return;
	m_modified=b;
	Gwen::Controls::DockBase *panel = m_main_window->GetRight();
	Gwen::Controls::DockedTabControl *tabs = (Gwen::Controls::DockedTabControl*)panel->GetTabControl();
	Gwen::Controls::TabButton *button= tabs->GetCurrentButton();
	Gwen::UnicodeString us=button->GetText();
	if(m_modified)
		us += L'*';
	else
		us.pop_back();
	button->SetText(us,true);
	tabs->UpdateTitleBar();
}

bool xcard_editor::_load_packet()
{
	if(!m_packet) 
		m_packet = new xcard_packet();
	if(!m_packet->load(m_file_path.c_str())) 
	{
		m_status_bar->SetText(L"加载牌组失败");
		return false;
	}
	m_controls.version->SetText(m_packet->version(),false);
	Gwen::UnicodeString index;
	xcard_data *cd;
	for(xcard_packet::iterator iter=m_packet->begin(), end=m_packet->end();
		iter!=end; ++iter) {
			cd = (xcard_data*)(iter->second);
			index = Gwen::Utility::Format(L"%04d",cd->index);	
			m_controls.card_list->AddItem(index)->SetCellText(1,cd->name);
	}
	Gwen::Controls::Base::List &children =m_controls.card_list->GetTable()->GetChildren();
	if(children.size())
		m_controls.card_list->SetSelectedRow((Gwen::Controls::Layout::TableRow*)*children.begin());
	m_status_bar->SetText(L"加载成功",false);
	return true;
}

void xcard_editor::_update_card_view(xcard_data *card)
{
	std::wstring us;
	if(!card) {
		wyc_print("clear card view");
		for(int i=0; i<PROP_COUNT; ++i)
			m_controls.props[i]->SetPropertyValue(us,false);
		return;
	}
	wyc_print("update card view [%04d]",card->index);
	uint2str(us,card->index);
	m_controls.props[PROP_INDEX]->SetPropertyValue(us,false);
	m_controls.props[PROP_NAME]->SetPropertyValue(card->name,false);
	uint2str(us,card->rarity);
	m_controls.props[PROP_RARITY]->SetPropertyValue(us,false);
	m_controls.props[PROP_TYPE]->SetPropertyValue(card->type,false);
	uint2str(us,card->cost);
	m_controls.props[PROP_COST]->SetPropertyValue(us,false);
	uint2str(us,card->max_hp);
	m_controls.props[PROP_MAXHP]->SetPropertyValue(us,false);
	uint2str(us,card->strength);
	m_controls.props[PROP_STRENGTH]->SetPropertyValue(us,false);
	m_controls.props[PROP_DESC]->SetPropertyValue(card->desc,false);


	std::wstring::size_type pos = card->avatar.find(L'@');
	std::wstring avatar_set, avatar_name;
	if(pos!=std::wstring::npos)
	{
		avatar_name = card->avatar.substr(0,pos);
		avatar_set = card->avatar.substr(pos+1);
	}
	else {
		avatar_name = card->avatar;
	}
	m_controls.props[PROP_AVATAR_SET]->SetPropertyValue(avatar_set,false);
	m_controls.props[PROP_AVATAR_NAME]->SetPropertyValue(avatar_name,false);
	if(!avatar_set.empty() && m_avatar_set!=avatar_set)
		_load_avatar_set(avatar_set);
}

void xcard_editor::_message_box_confirm()
{
	if(m_msgbox_cmd.empty())
		return;
	if(m_msgbox_cmd == "del_card") {
		Gwen::Controls::Layout::TableRow *item = m_controls.card_list->GetSelectedRow();
		if(item) {
			unsigned index = str2uint(item->GetCellContents(0)->GetText());
			m_controls.card_list->RemoveItem(item);
			m_packet->del_card(index);
			_select_card(0);
		}
	}
	m_msgbox_cmd.clear();
}

void xcard_editor::_select_card(unsigned id)
{
	if(m_card && m_card->index==id)
		return;
	if(id) 
		m_card = m_packet->get_card(id);
	else 
		m_card = 0;
	m_preview->initialize(m_card);
	m_preview->redraw();
	_update_card_view(m_card);
}

void xcard_editor::on_login_ok()
{
	m_status_bar->SetText(L"登录成功",false);
	m_controls.menu_login->SetText(L"登出");
	m_controls.menu_login->SetAction(m_handler,&xevent_handler::on_logout);
	m_controls.menu_login->SetDisabled(false);
}

void xcard_editor::on_login_fail(int err, const wchar_t *info)
{
	_message_box(L"登录",info);
	m_status_bar->SetText(L"登录失败",false);
	m_controls.menu_login->SetText(L"登录...");
	m_controls.menu_login->SetDisabled(false);
}

void xcard_editor::_load_avatar_set(const std::wstring &avatar_set)
{
	if(avatar_set.empty()) {
		m_status_bar->SetText(L"请输入路径");
		return;
	}
	std::string path;
	if(!wstr2str(path,avatar_set)) {
		m_status_bar->SetText(L"无效的文件路径");
		return;
	}
	m_avatar_set = avatar_set;
	m_avatar_name.clear();
	get_resource_path(path);
	xressvr *svr = get_resource_server();
	xtexture *avatar = (xtexture*)svr->async_request(xtexture::get_class()->id,path.c_str(),this,
		(xressvr::resource_callback_t)xcard_editor::_on_avatar_loaded);
	if(avatar && avatar->is_complete())
		xcard_editor::_on_avatar_loaded(this,avatar);
}

void xcard_editor::_on_avatar_loaded(xcard_editor *self, xtexture *avatar_tex)
{
	Gwen::Controls::ListBox *avatar_list = self->m_controls.avatar_list;
	avatar_list->Clear();
	if(avatar_tex==0) {
		self->m_status_bar->SetText(L"加载失败");
		return;
	}
	self->m_status_bar->SetText(L"加载成功");
	self->m_avatar = avatar_tex;
	const ximageset *imgset = avatar_tex->get_imageset();
	if(imgset) {
		for(ximageset::const_iterator iter=imgset->begin(), end=imgset->end(); 
			iter!=end; ++iter) {
			avatar_list->AddItem(iter->m_name);
		}
	}
}

//-------------------------------------------------------------------------------------------

xcard_editor::xevent_handler::xevent_handler( xcard_editor *editor) : m_editor(editor)
{
}

void xcard_editor::xevent_handler::on_msgbox_confirm(Gwen::Controls::Base *control, Gwen::Controls::Base* msgbox)
{
	((Gwen::Controls::WindowControl*)msgbox)->CloseButtonPressed(control);
	m_editor->_message_box_confirm();
}

void xcard_editor::xevent_handler::on_new_packet (Gwen::Controls::Base *pControl)
{
}

void xcard_editor::xevent_handler::on_load (Gwen::Controls::Base *pControl)
{
}

void xcard_editor::xevent_handler::on_save (Gwen::Controls::Base *pControl)
{
	xcard_packet *packet = m_editor->m_packet;
	if(!packet)
		return;
	if(!m_editor->is_modified()) {
		m_editor->m_status_bar->SetText(L"无更改");
		return;
	}
	if(!packet->save(m_editor->m_file_path.c_str()))
	{
		m_editor->_message_box(L"文件",L"保存牌组失败");
		return;
	}
	m_editor->m_status_bar->SetText(L"保存成功");
	m_editor->set_modified(false);
}

void xcard_editor::xevent_handler::on_quit (Gwen::Controls::Base *pControl)
{
	wyc_print("quit");
}

void xcard_editor::xevent_handler::on_login_dialog(Gwen::Controls::Base *pControl)
{
	int st = get_login_state();
	if(st==0)
		m_editor->_dialog_connection();
}

void xcard_editor::xevent_handler::on_login(Gwen::Controls::Base *pControl, Gwen::Controls::Base* dialog)
{
	Gwen::Controls::TextBox *usr = (Gwen::Controls::TextBox*)dialog->FindChildByName("usr",true);
	Gwen::Controls::SecureText *pwd = (Gwen::Controls::SecureText*)dialog->FindChildByName("pwd",true);
	if(!usr || !pwd)
		return;
	std::string account, password;
	const Gwen::UnicodeString &s1=usr->GetText();
	if(s1.empty())
		return;
	account.resize(s1.length());
	for(size_t i=0; i<s1.length(); ++i)
		account[i]=(char)s1[i];
	const Gwen::UnicodeString &s2=pwd->GetSecureText();
	password.resize(s2.length());
	for(size_t i=0; i<s2.length(); ++i)
		password[i]=(char)s2[i];
	((Gwen::Controls::WindowControl*)dialog)->CloseButtonPressed(pControl);
	m_editor->m_status_bar->SetText(L"正在登录...",false);
	m_editor->m_controls.menu_login->SetText(L"正在登录...");
	m_editor->m_controls.menu_login->SetDisabled(true);
	login(account,password);
}

void xcard_editor::xevent_handler::on_logout(Gwen::Controls::Base *pControl)
{
	m_editor->m_status_bar->SetText(L"正在登录...",false);
	m_editor->m_controls.menu_login->SetText(L"登录...");
	m_editor->m_controls.menu_login->SetDisabled(false);
	m_editor->m_controls.menu_login->SetAction(this,&xevent_handler::on_login_dialog);
	logout();
}

void xcard_editor::xevent_handler::on_packet_version(Gwen::Controls::Base *pControl)
{
	xcard_packet *packet = m_editor->m_packet;
	if(!packet)
		return;
	Gwen::Controls::TextBox* text = (Gwen::Controls::TextBox*) pControl;
	const Gwen::UnicodeString &us = text->GetText();
	if(us.empty())
		return;
	wchar_t ch;
	for(size_t i=0, cnt=us.length(); i<cnt; ++i)
	{
		ch = us[i];
		if((ch>=L'a' && ch<='z') || (ch>=L'A' && ch<='Z') || (ch>=L'0' && ch<=L'9'))
			continue;
		// invalid version name
		text->SetTextColor(Gwen::Colors::Red);
		return;
	}
	packet->set_version(us);
	text->MakeColorNormal();
	m_editor->set_modified(true);
}

void xcard_editor::xevent_handler::on_new_card(Gwen::Controls::Base *pControl)
{
	xcard_packet *packet = m_editor->m_packet;
	if(!packet)
		return;
	xcard_data *cd = packet->new_card();
	cd->name=L"未命名";
	cd->type=L"未知";
	cd->desc=L"写点什么吧...";
	m_editor->set_modified(true);

	Gwen::UnicodeString us = Gwen::Utility::Format(L"%04d",cd->index);	
	Gwen::Controls::ListBox *card_list = m_editor->m_controls.card_list;
	Gwen::Controls::Layout::TableRow *item = card_list->AddItem(us);
	item->SetCellText(1,cd->name);
	card_list->SetSelectedRow(item);
}

void xcard_editor::xevent_handler::on_del_card(Gwen::Controls::Base *pControl)
{
	Gwen::Controls::Layout::TableRow *item = m_editor->m_controls.card_list->GetSelectedRow();
	if(!item)
		return;
	m_editor->_message_box(L"确认",L"真的要删除这张卡片么？","del_card");
}

void xcard_editor::xevent_handler::on_search_card(Gwen::Controls::Base *pControl)
{
	const Gwen::Controls::TextBox* text = (Gwen::Controls::TextBox*) pControl;
	const Gwen::UnicodeString &search = text->GetText();
	Gwen::Controls::ListBox *card_list = m_editor->m_controls.card_list;
	Gwen::Controls::Layout::Table *tab= card_list->GetTable();
	Gwen::Controls::Base::List &children = tab->GetChildren();
	Gwen::Controls::Layout::TableRow *row;
	const Gwen::Controls::Label *label;
	bool found = false;
	for(Gwen::Controls::Base::List::iterator iter = children.begin(), end = children.end();
		iter!=end; ++iter) 
	{
		row = (Gwen::Controls::Layout::TableRow*)*iter;
		label = row->GetCellContents(0);
		assert(label);
		if(label->GetText().find(search)!=std::wstring::npos) {
			found = true;
			break;
		}
		label = row->GetCellContents(1);
		assert(label);
		if(label->GetText().find(search)!=std::wstring::npos) {
			found = true;
			break;
		}
	}
	if(found) {
		Gwen::Controls::Layout::TableRow *current = card_list->GetSelectedRow();
		if(current!=row) 
			card_list->SetSelectedRow(row);
	}
}

void xcard_editor::xevent_handler::on_select_card(Gwen::Controls::Base *pControl)
{
	Gwen::Controls::ListBox* list = (Gwen::Controls::ListBox*)pControl;
	Gwen::Controls::Layout::TableRow *item = list->GetSelectedRow();
	assert(item);
	unsigned index = str2uint(item->GetCellContents(0)->GetText());
	m_editor->_select_card(index);
}

bool _is_positive_integer(const Gwen::UnicodeString &s)
{
	wchar_t ch;
	for(size_t i=0, cnt=s.length(); i<cnt; ++i) {
		ch = s[i];
		if(ch==L'-' || ch==L'.')
			return false;
		if(ch>=L'0' && ch<=L'9')
			continue;
		return false;
	}
	return true;
}


void xcard_editor::xevent_handler::on_index_changed(Gwen::Controls::Base* pControl)
{
	if(!m_editor->m_card)
		return;
	std::wstring s;
	uint2str(s,m_editor->m_card->index);
	m_editor->m_controls.props[PROP_INDEX]->SetPropertyValue(s,false);
}

void xcard_editor::xevent_handler::on_name_changed( Gwen::Controls::Base* pControl )
{
	if(!m_editor->m_card)
		return;
	m_editor->m_card->name=m_editor->m_controls.props[PROP_NAME]->GetPropertyValue();
	m_editor->m_preview->redraw();
	m_editor->set_modified(true);
	Gwen::Controls::Layout::TableRow *item = m_editor->m_controls.card_list->GetSelectedRow();
	assert(item);
	item->SetCellText(1,m_editor->m_card->name);
}

void xcard_editor::xevent_handler::on_rarity_changed(Gwen::Controls::Base* pControl)
{
	if(!m_editor->m_card)
		return;
	Gwen::Controls::Property::Text *text_prop = (Gwen::Controls::Property::Text*)m_editor->m_controls.props[PROP_RARITY];
	Gwen::UnicodeString us = text_prop->GetPropertyValue();
	if(us.empty())
		return;
	if(!_is_positive_integer(us))
	{
		text_prop->m_TextBox->SetTextColor(Gwen::Colors::Red);
		m_editor->m_status_bar->SetText(L"只能输入正整数");
		return;
	}
	unsigned value = str2uint(us);
	if(value>=RARITY_COUNT)
	{
		text_prop->m_TextBox->SetTextColor(Gwen::Colors::Red);
		m_editor->m_status_bar->SetText(L"数值超出范围");
		return;
	}
	text_prop->m_TextBox->MakeColorNormal();
	m_editor->m_card->rarity=value;
	m_editor->m_preview->redraw();
	m_editor->set_modified(true);
}

void xcard_editor::xevent_handler::on_type_changed(Gwen::Controls::Base* pControl)
{
	if(!m_editor->m_card)
		return;
	m_editor->m_card->type=m_editor->m_controls.props[PROP_TYPE]->GetPropertyValue();
	m_editor->m_preview->redraw();
	m_editor->set_modified(true);
}

void xcard_editor::xevent_handler::on_cost_changed(Gwen::Controls::Base* pControl)
{
	if(!m_editor->m_card)
		return;
	Gwen::Controls::Property::Text *text_prop = (Gwen::Controls::Property::Text*)m_editor->m_controls.props[PROP_COST];
	Gwen::UnicodeString us = text_prop->GetPropertyValue();
	if(us.empty())
		return;
	if(!_is_positive_integer(us))
	{
		text_prop->m_TextBox->SetTextColor(Gwen::Colors::Red);
		m_editor->m_status_bar->SetText(L"只能输入正整数");
		return;
	}
	unsigned value = str2uint(us);
	if(value>999)
	{
		text_prop->m_TextBox->SetTextColor(Gwen::Colors::Red);
		m_editor->m_status_bar->SetText(L"数值超出范围");
		return;
	}
	text_prop->m_TextBox->MakeColorNormal();
	m_editor->m_card->cost=value;
	m_editor->m_preview->redraw();
	m_editor->set_modified(true);
}

void xcard_editor::xevent_handler::on_maxhp_changed(Gwen::Controls::Base* pControl)
{
	if(!m_editor->m_card)
		return;
	Gwen::Controls::Property::Text *text_prop = (Gwen::Controls::Property::Text*)m_editor->m_controls.props[PROP_MAXHP];
	Gwen::UnicodeString us = text_prop->GetPropertyValue();
	if(us.empty())
		return;
	if(!_is_positive_integer(us))
	{
		text_prop->m_TextBox->SetTextColor(Gwen::Colors::Red);
		m_editor->m_status_bar->SetText(L"只能输入正整数");
		return;
	}
	unsigned value = str2uint(us);
	if(value>99)
	{
		text_prop->m_TextBox->SetTextColor(Gwen::Colors::Red);
		m_editor->m_status_bar->SetText(L"数值超出范围");
		return;
	}
	text_prop->m_TextBox->MakeColorNormal();
	m_editor->m_card->max_hp=value;
	m_editor->m_preview->redraw();
	m_editor->set_modified(true);
}

void xcard_editor::xevent_handler::on_strength_changed(Gwen::Controls::Base* pControl)
{
	if(!m_editor->m_card)
		return;
	if(!m_editor->m_card)
		return;
	Gwen::Controls::Property::Text *text_prop = (Gwen::Controls::Property::Text*)m_editor->m_controls.props[PROP_STRENGTH];
	Gwen::UnicodeString us = text_prop->GetPropertyValue();
	if(us.empty())
		return;
	if(!_is_positive_integer(us))
	{
		text_prop->m_TextBox->SetTextColor(Gwen::Colors::Red);
		m_editor->m_status_bar->SetText(L"只能输入正整数");
		return;
	}
	unsigned value = str2uint(us);
	if(value>99)
	{
		text_prop->m_TextBox->SetTextColor(Gwen::Colors::Red);
		m_editor->m_status_bar->SetText(L"数值超出范围");
		return;
	}
	text_prop->m_TextBox->MakeColorNormal();
	m_editor->m_card->strength=value;
	m_editor->m_preview->redraw();
	m_editor->set_modified(true);
}

void xcard_editor::xevent_handler::on_desc_changed(Gwen::Controls::Base* pControl)
{
	if(!m_editor->m_card)
		return;
	m_editor->m_card->desc=m_editor->m_controls.props[PROP_DESC]->GetPropertyValue();
	m_editor->m_preview->redraw();
	m_editor->set_modified(true);
}

void xcard_editor::xevent_handler::on_load_avatar(Gwen::Controls::Base* pControl, Gwen::Controls::Base* textBox)
{
	const Gwen::UnicodeString &text = ((Gwen::Controls::TextBox*)textBox)->GetText();
	m_editor->_load_avatar_set(text);
}

void xcard_editor::xevent_handler::on_select_avatar(Gwen::Controls::Base* pControl)
{
	if(!m_editor->m_card)
		return;
	Gwen::Controls::ListBox* list = (Gwen::Controls::ListBox*)pControl;
	Gwen::Controls::Layout::TableRow *item = list->GetSelectedRow();
	assert(item);
	m_editor->m_avatar_name = item->GetText(0);
	wyc::format(m_editor->m_card->avatar,L"%s@%s",m_editor->m_avatar_name.c_str(),m_editor->m_avatar_set.c_str());
	m_editor->m_preview->redraw();
	m_editor->set_modified(true);
	m_editor->m_controls.props[PROP_AVATAR_SET]->SetPropertyValue(m_editor->m_avatar_set,false);
	m_editor->m_controls.props[PROP_AVATAR_NAME]->SetPropertyValue(m_editor->m_avatar_name,false);
}

}; // namespace wyc
