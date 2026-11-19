#include "common.h"
#ifdef DEBUGMENU
#include "RwHelper.h"
#include "Pad.h"
#include "ControllerConfig.h"
#include "Timer.h"
#include "Frontend.h"
#include "CutsceneMgr.h"
#include "Sprite2d.h"
#include <string>
#include "rtcharse.h"
#include "re3_inttypes.h"
#include "debugmenu.h"
#include <new>

#ifdef _WIN32
#define snprintf _snprintf

#define strdup _strdup
#endif


// Font stuff
struct Pt
{
	int x, y;
};

enum MenuFontStyle
{
	MENUFONT_NORMAL,
	MENUFONT_SEL_ACTIVE,
	MENUFONT_SEL_INACTIVE,
	MENUFONT_MOUSE
};

RtCharset *fontStyles[4];
RtCharsetDesc fontDesc;
int fontscale = 1;	// not supported right now

Pt
fontGetStringSize(const char *s)
{
	Pt sz = { 0, 0 };
	int x;
	char c;
	sz.y = fontDesc.height*fontscale;	// always assume one line;
	x = 0;
	while(c = *s++){
		if(c == '\n'){
			sz.y += fontDesc.height*fontscale;
			if(x > sz.x)
				sz.x = x;
			x = 0;
		}else
			x += fontDesc.width*fontscale;
	}
	if(x > sz.x)
		sz.x = x;
	return sz;
}

Pt
fontPrint(const char *s, float x, float y, int style)
{
	RtCharsetPrintBuffered(fontStyles[style], s, x, y, false);
	return fontGetStringSize(s);
}

int
fontGetLen(int len)
{
	return len*fontDesc.width*fontscale;
}


void
createMenuFont(void)
{
	OpenCharsetSafe();

	RwRGBA fg_normal = { 255, 255, 255, 255 };
	RwRGBA bg_normal = { 255, 255, 255, 0 };
	fontStyles[MENUFONT_NORMAL] = RtCharsetCreate(&fg_normal, &bg_normal);
	assert(fontStyles[MENUFONT_NORMAL]);

	RwRGBA fg_sel_active = { 200, 200, 200, 255 };
	RwRGBA bg_sel_active = { 132, 132, 132, 255 };
	fontStyles[MENUFONT_SEL_ACTIVE] = RtCharsetCreate(&fg_sel_active, &bg_sel_active);
	assert(fontStyles[MENUFONT_SEL_ACTIVE]);

	RwRGBA fg_sel_inactive = { 200, 200, 200, 255 };
	RwRGBA bg_sel_inactive = { 200, 200, 200, 0 };
	fontStyles[MENUFONT_SEL_INACTIVE] = RtCharsetCreate(&fg_sel_inactive, &bg_sel_inactive);
	assert(fontStyles[MENUFONT_SEL_INACTIVE]);

	RwRGBA fg_mouse = { 255, 255, 255, 255 };
	RwRGBA bg_mouse = { 132, 132, 132, 255 };
	fontStyles[MENUFONT_MOUSE] = RtCharsetCreate(&fg_mouse, &bg_mouse);
	assert(fontStyles[MENUFONT_MOUSE]);

	RtCharsetGetDesc(fontStyles[MENUFONT_NORMAL], &fontDesc);
}

void
destroyMenuFont(void)
{
	RtCharsetDestroy(fontStyles[MENUFONT_NORMAL]);
	fontStyles[MENUFONT_NORMAL] = nil;
	RtCharsetDestroy(fontStyles[MENUFONT_SEL_ACTIVE]);
	fontStyles[MENUFONT_SEL_ACTIVE] = nil;
	RtCharsetDestroy(fontStyles[MENUFONT_SEL_INACTIVE]);
	fontStyles[MENUFONT_SEL_INACTIVE] = nil;
	RtCharsetDestroy(fontStyles[MENUFONT_MOUSE]);
	fontStyles[MENUFONT_MOUSE] = nil;
}






enum EntryType
{
	MENUEMPTY = 0,
	MENUSUB,
	MENUVAR,

	MENUVAR_INT,
	MENUVAR_FLOAT,
	MENUVAR_CMD,
};

struct Menu
{
	Menu *parent;
	const char *name;
	RwRect r;
	MenuEntry *entries;
	int numEntries;
	int maxValWidth;

	MenuEntry *findEntry(const char *entryname);
	void insertEntrySorted(MenuEntry *entry);
	void appendEntry(MenuEntry *entry);

	bool isScrollingUp, isScrollingDown;
	int scrollStart;
	int numVisible;
	RwRect scrollUpR, scrollDownR;
	void scroll(int off);

	int selection;
	MenuEntry *selectedEntry;	// updated by update
	void changeSelection(int newsel);

	void update(void);
	void draw(void);
	Menu(void){ memset(this, 0, sizeof(Menu)); }
	~Menu(void);
};
extern Menu toplevel;

struct MenuEntry_Sub : MenuEntry
{
	Menu *submenu;

	MenuEntry_Sub(const char *name, Menu *menu);
	~MenuEntry_Sub(void) { delete submenu; }
};

struct MenuEntry_Var : MenuEntry
{
	int maxvallen;
	int vartype;
	bool wrapAround;

	virtual void processInput() = 0;
	int getValWidth(void) { return maxvallen; }
	virtual void getValStr(char *str, int len) = 0;
	MenuEntry_Var(const char *name, int type);
};

struct MenuEntry_Int : MenuEntry_Var
{
	virtual void setStrings(const char **strings) = 0;
	virtual int findStringLen(void) = 0;
	MenuEntry_Int(const char *name);
};

#define INTTYPES \
	X(Int8, int8, 4, "%4" PRId8) \
	X(Int16, int16, 6, "%6" PRId16) \
	X(Int32, int32, 11, "%11" PRId32) \
	X(Int64, int64, 21, "%21" PRId64) \
	X(UInt8, uint8, 4, "%4" PRIu8) \
	X(UInt16, uint16, 6, "%6" PRIu16) \
	X(UInt32, uint32, 11, "%11" PRIu32) \
	X(UInt64, uint64, 21, "%21" PRIu64)
#define FLOATTYPES \
	X(Float32, float, 11, "%11.3f") \
	X(Float64, double, 11, "%11.3lf")

#define X(NAME, TYPE, MAXLEN, FMT) \
struct MenuEntry_##NAME : MenuEntry_Int														       \
{																		       \
	TYPE *variable;																       \
	TYPE lowerBound, upperBound;														       \
	TYPE step;																       \
	TriggerFunc triggerFunc;														       \
	const char *fmt;															       \
	const char **strings;															       \
																		       \
	void processInput();														       \
	void getValStr(char *str, int len);													       \
																		       \
	void setStrings(const char **strings);													       \
	int findStringLen(void);														       \
	MenuEntry_##NAME(const char *name, TYPE *ptr, TriggerFunc triggerFunc, TYPE step, TYPE lowerBound, TYPE upperBound, const char **strings);     \
};
INTTYPES
#undef X

#define X(NAME, TYPE, MAXLEN, FMT) \
struct MenuEntry_##NAME : MenuEntry_Var												 \
{																 \
	TYPE *variable;														 \
	TYPE lowerBound, upperBound;												 \
	TYPE step;														 \
	TriggerFunc triggerFunc;												 \
	const char *fmt;													 \
																 \
	void processInput();												 \
	void getValStr(char *str, int len);											 \
																 \
	MenuEntry_##NAME(const char *name, TYPE *ptr, TriggerFunc triggerFunc, TYPE step, TYPE lowerBound, TYPE upperBound);	 \
};
FLOATTYPES
#undef X

struct MenuEntry_Cmd : MenuEntry_Var
{
	TriggerFunc triggerFunc;

	void processInput();
	void getValStr(char *str, int len);

	MenuEntry_Cmd(const char *name, TriggerFunc triggerFunc);
};


Menu *findMenu(const char *name);



#define MUHKEYS \
	X(leftjustdown, rsLEFT) \
	X(rightjustdown, rsRIGHT) \
	X(upjustdown, rsUP) \
	X(downjustdown, rsDOWN) \
	X(pgupjustdown, rsPGUP) \
	X(pgdnjustdown, rsPGDN)

#define REPEATDELAY 700
#define REPEATINTERVAL 50
#define X(var, keycode) static int var;
MUHKEYS
#undef X
static int downtime;
static int repeattime;
static int lastkeydown;
static int *keyptr;

static int menuOn;
static int menuInitialized;
static int screenWidth, screenHeight;
static RwRaster *arrow;

static constexpr int panelLeft = 20;
static constexpr int panelTop = 36;
static constexpr int panelWidth = 384;
static constexpr int panelHeader = 54;
static constexpr int panelFooter = 36;
static constexpr int rowLeading = 8;
static constexpr int rowPadding = 12;


void drawArrow(RwRect r, int direction, int style);

Menu toplevel;
Menu *activeMenu = &toplevel;

#define KEYJUSTDOWN(k) ControlsManager.GetIsKeyboardKeyJustDown((RsKeyCodes)k)
#define KEYDOWN(k) ControlsManager.GetIsKeyboardKeyDown((RsKeyCodes)k)


/*
 * MenuEntry
 */

MenuEntry::MenuEntry(const char *name)
{
	this->type = MENUEMPTY;
	this->name = strdup(name);
	this->next = nil;
	this->menu = nil;
}

MenuEntry_Sub::MenuEntry_Sub(const char *name, Menu *menu)
: MenuEntry(name)
{
	this->type = MENUSUB;
	this->submenu = menu;
}

MenuEntry_Var::MenuEntry_Var(const char *name, int vartype)
: MenuEntry(name)
{
	this->type = MENUVAR;
	this->vartype = vartype;
	this->maxvallen = 0;
	this->wrapAround = false;
}

/*
 * *****************************
 * MenuEntry_Int
 * *****************************
 */

MenuEntry_Int::MenuEntry_Int(const char *name)
: MenuEntry_Var(name, MENUVAR_INT)
{
}

#define X(NAME, TYPE, MAXLEN, FMT) \
int								     \
MenuEntry_##NAME::findStringLen(void){				     \
	TYPE i;							     \
	int len, maxlen = 0;					     \
	for(i = this->lowerBound; i <= this->upperBound; i++){	     \
		len = strlen(this->strings[i-this->lowerBound]);     \
		if(len > maxlen)				     \
			maxlen = len;				     \
	}							     \
	return maxlen;						     \
}								     \
void								     \
MenuEntry_##NAME::processInput()				     \
{								     \
	TYPE v, oldv;						     \
	int overflow = 0;					     \
	int underflow = 0;					     \
								     \
	v = *this->variable;					     \
	oldv = v;						     \
								     \
	if(leftjustdown){					     \
		v -= this->step;				     \
		if(v > oldv)					     \
			underflow = 1;				     \
	}							     \
	if(rightjustdown){					     \
		v += this->step;				     \
		if(v < oldv)					     \
			overflow = 1;				     \
	}							     \
	if(this->wrapAround){					     \
		if(v > this->upperBound || overflow) v = this->lowerBound;	     \
		if(v < this->lowerBound || underflow) v = this->upperBound;	     \
	}else{									     \
		if(v > this->upperBound || overflow) v = this->upperBound;	     \
		if(v < this->lowerBound || underflow) v = this->lowerBound;	     \
	}							     \
								     \
	*this->variable = v;					     \
	if(oldv != v && this->triggerFunc)			     \
		this->triggerFunc();				     \
}								     \
void											       \
MenuEntry_##NAME::getValStr(char *str, int len)						       \
{											       \
	static char tmp[20];								       \
	if(this->strings){								       \
		snprintf(tmp, 20, "%%%ds", this->maxvallen);				       \
		if(*this->variable < this->lowerBound || *this->variable > this->upperBound){  \
			snprintf(str, len, "ERROR");					       \
			return;								       \
		}									       \
		snprintf(str, len, tmp, this->strings[*this->variable-this->lowerBound]);      \
	}else										       \
		snprintf(str, len, this->fmt, *this->variable);				       \
}											       \
void																			   \
MenuEntry_##NAME::setStrings(const char **strings)													   \
{																			   \
	this->strings = strings;															   \
	if(this->strings)																   \
		this->maxvallen = findStringLen();													   \
}																			   \
MenuEntry_##NAME::MenuEntry_##NAME(const char *name, TYPE *ptr, TriggerFunc triggerFunc, TYPE step, TYPE lowerBound, TYPE upperBound, const char **strings)   \
: MenuEntry_Int(name)																	   \
{																			   \
	this->variable = ptr;																   \
	this->step = step;																   \
	this->lowerBound = lowerBound;															   \
	this->upperBound = upperBound;															   \
	this->triggerFunc = triggerFunc;														   \
	this->maxvallen = MAXLEN;															   \
	this->fmt = FMT;																   \
	this->setStrings(strings);																   \
}
INTTYPES
#undef X

/*
 * *****************************
 * MenuEntry_Float
 * *****************************
 */

#define X(NAME, TYPE, MAXLEN, FMT) \
MenuEntry_##NAME::MenuEntry_##NAME(const char *name, TYPE *ptr, TriggerFunc triggerFunc, TYPE step, TYPE lowerBound, TYPE upperBound)	     \
: MenuEntry_Var(name, MENUVAR_FLOAT)															     \
{																	     \
	this->variable = ptr;														     \
	this->step = step;														     \
	this->lowerBound = lowerBound;													     \
	this->upperBound = upperBound;													     \
	this->triggerFunc = triggerFunc;												     \
	this->maxvallen = MAXLEN;													     \
	this->fmt = FMT;														     \
}																	     \
void																	     \
MenuEntry_##NAME::getValStr(char *str, int len)												     \
{																	     \
	snprintf(str, len, this->fmt, *this->variable);											     \
}																	     \
void																	     \
MenuEntry_##NAME::processInput()													     \
{																	     \
	float v, oldv;															     \
	int overflow = 0;					     \
	int underflow = 0;					     \
																	     \
	v = *this->variable;														     \
	oldv = v;															     \
																	     \
	if(leftjustdown){					     \
		v -= this->step;				     \
		if(v > oldv)					     \
			underflow = 1;				     \
	}							     \
	if(rightjustdown){					     \
		v += this->step;				     \
		if(v < oldv)					     \
			overflow = 1;				     \
	}							     \
	if(this->wrapAround){					     \
		if(v > this->upperBound || overflow) v = this->lowerBound;	     \
		if(v < this->lowerBound || underflow) v = this->upperBound;	     \
	}else{									     \
		if(v > this->upperBound || overflow) v = this->upperBound;	     \
		if(v < this->lowerBound || underflow) v = this->lowerBound;	     \
	}							     \
																	     \
	*this->variable = v;														     \
	if(oldv != v && this->triggerFunc)												     \
		this->triggerFunc();													     \
}

FLOATTYPES
#undef X

/*
 * *****************************
 * MenuEntry_Cmd
 * *****************************
 */

void
MenuEntry_Cmd::processInput()
{
	if(this->triggerFunc && (leftjustdown || rightjustdown))
		this->triggerFunc();
}

void
MenuEntry_Cmd::getValStr(char *str, int len)
{
	strncpy(str, "<", len);
}

MenuEntry_Cmd::MenuEntry_Cmd(const char *name, TriggerFunc triggerFunc)
: MenuEntry_Var(name, MENUVAR_CMD)
{
	this->maxvallen = 1;
	this->triggerFunc = triggerFunc;
}


/*
 * *****************************
 * Menu
 * *****************************
 */

void
Menu::scroll(int off) {
	if(isScrollingUp && off < 0)
		scrollStart += off;
	if(isScrollingDown && off > 0)
		scrollStart += off;
	if(scrollStart < 0) scrollStart = 0;
	if(scrollStart > numEntries-numVisible) scrollStart = numEntries-numVisible;
}

void
Menu::changeSelection(int newsel){
	selection = newsel;
	if(selection < 0) selection = 0;
	if(selection >= numEntries) selection = numEntries-1;
	if(selection < scrollStart) scrollStart = selection;
	if(selection >= scrollStart+numVisible) scrollStart = selection-numVisible+1;
}

MenuEntry*
Menu::findEntry(const char *entryname)
{
	MenuEntry *m;
	for(m = this->entries; m; m = m->next)
		if(strcmp(entryname, m->name) == 0)
			return m;
	return nil;
}

void
Menu::insertEntrySorted(MenuEntry *entry)
{
	MenuEntry **mp;
	int cmp;
	for(mp = &this->entries; *mp; mp = &(*mp)->next){
		cmp = strcmp(entry->name, (*mp)->name);
		if(cmp == 0)
			return;
		if(cmp < 0)
			break;
	}
	entry->next = *mp;
	*mp = entry;
	entry->menu = this;
	this->numEntries++;
}

void
Menu::appendEntry(MenuEntry *entry)
{
	MenuEntry **mp;
	for(mp = &this->entries; *mp; mp = &(*mp)->next);
	entry->next = *mp;
	*mp = entry;
	entry->menu = this;
	this->numEntries++;
}

void
Menu::update(void)
{
	const int rowHeight = fontDesc.height + rowLeading;
	const int capacity = Max(1, (r.h - 32) / rowHeight);
	numVisible = Min(numEntries, capacity);
	scrollStart = Clamp(scrollStart, 0, numEntries - numVisible);
	isScrollingUp = scrollStart > 0;
	isScrollingDown = scrollStart + numVisible < numEntries;

	selectedEntry = nil;
	maxValWidth = 0;
	int i = 0;
	for (MenuEntry *entry = entries; entry; entry = entry->next, ++i) {
		entry->r = {r.x, r.y + 16 + (i - scrollStart) * rowHeight, r.w, rowHeight};
		if (i == selection)
			selectedEntry = entry;
		if (entry->type == MENUVAR)
			maxValWidth = Max(maxValWidth, fontGetLen(((MenuEntry_Var *)entry)->getValWidth()));
	}

	scrollUpR = {r.x, r.y, r.w, 16};
	scrollDownR = {r.x, r.y + 16 + numVisible * rowHeight, r.w, 16};
}

static void
DrawTrainerRect(float x, float y, float width, float height, const CRGBA &colour)
{
	CSprite2d::DrawRect(CRect(x, y, x + width, y + height), colour);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
}

static void
PrintTrainerLabel(const char *label, int x, int y, int width)
{
	const int maxChars = width / fontGetLen(1);
	if (maxChars < 4)
		return;
	std::string clipped(label);
	if (clipped.size() > size_t(maxChars)) {
		clipped.resize(maxChars - 3);
		clipped += "...";
	}
	fontPrint(clipped.c_str(), x, y, MENUFONT_NORMAL);
}

void
Menu::draw(void)
{
	int i = 0;
	for (MenuEntry *entry = entries; entry && i < scrollStart + numVisible; entry = entry->next, ++i) {
		if (i < scrollStart)
			continue;
		const RwRect &row = entry->r;
		if (i == selection) {
			RtCharsetBufferFlush();
			DrawTrainerRect(row.x, row.y, row.w, row.h, CRGBA(44, 97, 107, 250));
			DrawTrainerRect(row.x, row.y, 3, row.h, CRGBA(231, 192, 105, 255));
		}

		const int textY = row.y + rowLeading / 2;
		const int valueWidth = Min(maxValWidth, row.w / 3);
		PrintTrainerLabel(entry->name, row.x + rowPadding, textY, row.w - 3 * rowPadding - valueWidth);
		if (entry->type == MENUSUB) {
			fontPrint(">", row.x + row.w - rowPadding - fontGetLen(1), textY, MENUFONT_NORMAL);
		} else if (entry->type == MENUVAR) {
			char value[100];
			((MenuEntry_Var *)entry)->getValStr(value, sizeof(value));
			value[sizeof(value) - 1] = '\0';
			const int width = Min(fontGetStringSize(value).x, valueWidth);
			PrintTrainerLabel(value, row.x + row.w - rowPadding - width, textY, Max(width, fontGetLen(4)));
		}
	}

	RtCharsetBufferFlush();
	if (isScrollingUp)
		drawArrow(scrollUpR, -1, false);
	if (isScrollingDown)
		drawArrow(scrollDownR, 1, false);
}

Menu::~Menu(void)
{
	MenuEntry *e, *next;
	for(e = entries; e; e = next){
		next = e->next;
		delete e;
	}
	memset(this, 0, sizeof(Menu));
}

Menu*
findMenu(const char *name)
{
	Menu *m;
	MenuEntry *e;
	char *tmppath = strdup(name);
	char *next, *curname;

	curname = tmppath;
	next = curname;

	m = &toplevel;
	while(*next){
		curname = next;
		while(*next){
			if(*next == '|'){
				*next++ = '\0';
				break;
			}
			next++;
		}
		e = m->findEntry(curname);
		if(e){
			// return an error if the entry exists but isn't a menu
			if(e->type != MENUSUB){
				free(tmppath);
				return nil;
			}
			m = ((MenuEntry_Sub*)e)->submenu;
		}else{
			// Create submenus that don't exist yet
			Menu *submenu = new Menu();
			submenu->parent = m;
			MenuEntry *me = new MenuEntry_Sub(curname, submenu);
			submenu->name = me->name;
			// Don't sort submenus outside the toplevel menu
			if(m == &toplevel)
				m->insertEntrySorted(me);
			else
				m->appendEntry(me);
			m = submenu;
		}
	}

	free(tmppath);
	return m;
}

/*
 * ****************
 * debug menu
 * ****************
 */

static uint8 arrowPx[] = {
#include "arrow.inc"
};

void
DebugMenuInit(void)
{
	createMenuFont();

	RwInt32 w, h, d, flags;
	RwImage *img = RwImageCreate(32, 16, 32);
	assert(img);
	RwImageSetPixels(img, arrowPx);
	RwImageSetStride(img, RwImageGetWidth(img)*4);
	RwImageFindRasterFormat(img, rwRASTERTYPETEXTURE, &w, &h, &d, &flags);
	arrow = RwRasterCreate(w, h, d, flags);
	arrow = RwRasterSetFromImage(arrow, img);
	assert(arrow);
	RwImageDestroy(img);


	menuInitialized = true;
}

void
DebugMenuShutdown(void)
{
	DebugMenuClose();
	if(menuInitialized){
		destroyMenuFont();
		RwRasterDestroy(arrow);
		arrow = nil;

		toplevel.~Menu();
		new (&toplevel) Menu();

		activeMenu = &toplevel;
	}
	menuInitialized = false;
}

void
processInput(void)
{
	int shift = KEYDOWN(rsRSHIFT) || KEYDOWN(rsLSHIFT);
#define X(var, keycode) var = KEYJUSTDOWN(keycode);
	MUHKEYS
#undef X

	// Implement auto-repeat
#define X(var, keycode) \
	if(var){ \
		repeattime = downtime = CTimer::GetTimeInMillisecondsPauseMode(); \
		lastkeydown = keycode; \
		keyptr = &var; \
	}
	MUHKEYS
#undef X
	if(lastkeydown){
		if(KEYDOWN(lastkeydown)){
			int curtime = CTimer::GetTimeInMillisecondsPauseMode();
			if(curtime - downtime > REPEATDELAY){
				if(curtime - repeattime > REPEATINTERVAL){
					repeattime = curtime;
					*keyptr = 1;
				}
			}
		}else{
			lastkeydown = 0;
		}
	}

	if (pgupjustdown)
		activeMenu->scroll(shift ? -5 : -1);
	if (pgdnjustdown)
		activeMenu->scroll(shift ? 5 : 1);
	if (downjustdown)
		activeMenu->changeSelection(activeMenu->selection + (shift ? 5 : 1));
	if (upjustdown)
		activeMenu->changeSelection(activeMenu->selection - (shift ? 5 : 1));

	activeMenu->update();
	MenuEntry *entry = activeMenu->selectedEntry;
	if (!entry)
		return;
	if (KEYJUSTDOWN(rsENTER) && entry->type == MENUSUB) {
		activeMenu = ((MenuEntry_Sub *)entry)->submenu;
		return;
	}
	if (entry->type == MENUVAR) {
		MenuEntry_Var *value = (MenuEntry_Var *)entry;
		if (KEYJUSTDOWN(rsENTER) && value->vartype == MENUVAR_CMD) {
			MenuEntry_Cmd *command = (MenuEntry_Cmd *)value;
			if (command->triggerFunc)
				command->triggerFunc();
		} else {
			value->processInput();
		}
	}
}

static void
LayoutTrainer()
{
	activeMenu->r = {
		panelLeft, panelTop + panelHeader, Min(panelWidth, Max(1, screenWidth - 2 * panelLeft)),
		Max(48, Min(screenHeight - panelTop - panelHeader - panelFooter - 20,
		            32 + 12 * (fontDesc.height + rowLeading)))
	};
	activeMenu->update();
}

void
DebugMenuClose(void)
{
	if (!menuOn)
		return;
	menuOn = false;
	activeMenu = &toplevel;
	lastkeydown = 0;
}

bool
DebugMenuIsOpen(void)
{
	return menuOn != 0;
}

void
DebugMenuProcess(void)
{
	CPad *pad = CPad::GetPad(0);
	if (CTimer::GetIsPaused() || FrontEndMenuManager.m_bMenuActive ||
	    FrontEndMenuManager.m_bWantToRestart || FrontEndMenuManager.m_bWantToLoad ||
	    CCutsceneMgr::IsCutsceneProcessing()) {
		DebugMenuClose();
		return;
	}
	if (pad->GetFJustDown(7)) { // F8
		if (menuOn) {
			pad->ConsumeEscapeJustDown();
			DebugMenuClose();
			return;
		}
		if (!pad->ArePlayerControlsDisabled())
			menuOn = true;
	}
	if (!menuOn)
		return;
	// Consume local input while the trainer has focus without setting
	// the global player-control flag consulted by world and AI code.
	pad->NewState.Clear();
	pad->OldState.Clear();
	pad->NewMouseControllerState.Clear();
	if (pad->GetEscapeJustDown() || pad->GetBackspaceJustDown()) {
		pad->ConsumeEscapeJustDown();
		if (activeMenu->parent) {
			activeMenu = activeMenu->parent;
		} else {
			DebugMenuClose();
		}
		return;
	}

	if (!menuInitialized)
		DebugMenuInit();
	screenWidth = SCREEN_WIDTH;
	screenHeight = SCREEN_HEIGHT;
	LayoutTrainer();
	processInput();
}

void
DebugMenuRender(void)
{
	if (!menuOn || FrontEndMenuManager.m_bMenuActive || CTimer::GetIsPaused())
		return;

	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, 0);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, 0);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, 0);
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);

	RwCamera *camera = RwCameraGetCurrentCamera();
	screenWidth = RwRasterGetWidth(RwCameraGetRaster(camera));
	screenHeight = RwRasterGetHeight(RwCameraGetRaster(camera));
	LayoutTrainer();

	const RwRect &list = activeMenu->r;
	DrawTrainerRect(panelLeft, panelTop, list.w, panelHeader + list.h + panelFooter, CRGBA(11, 18, 23, 235));
	DrawTrainerRect(panelLeft, panelTop, list.w, 28, CRGBA(28, 51, 59, 255));
	DrawTrainerRect(panelLeft, panelTop + 28, list.w, 2, CRGBA(231, 192, 105, 255));
	fontPrint("LIBERTY CITY TRAINER", panelLeft + rowPadding, panelTop + 8, MENUFONT_NORMAL);

	std::string path;
	for (const Menu *menu = activeMenu; menu->parent; menu = menu->parent) {
		if (!path.empty())
			path = " / " + path;
		path = menu->name + path;
	}
	char count[32];
	snprintf(count, sizeof(count), "%d / %d",
	         activeMenu->numEntries ? activeMenu->selection + 1 : 0, activeMenu->numEntries);
	const int countWidth = fontGetStringSize(count).x;
	PrintTrainerLabel(path.empty() ? "Categories" : path.c_str(), panelLeft + rowPadding,
	                  panelTop + 37, list.w - countWidth - 3 * rowPadding);
	fontPrint(count, panelLeft + list.w - rowPadding - countWidth, panelTop + 37, MENUFONT_NORMAL);
	RtCharsetBufferFlush();

	activeMenu->draw();
	PrintTrainerLabel("Arrows: Navigate   Enter: Open / Run", panelLeft + rowPadding,
	                  list.y + list.h + 1, list.w - 2 * rowPadding);
	PrintTrainerLabel("Esc / Backspace: Back   F8: Close", panelLeft + rowPadding,
	                  list.y + list.h + 13, list.w - 2 * rowPadding);
	RtCharsetBufferFlush();
}

void
drawArrow(RwRect r, int direction, int style)
{
	static RwImVertexIndex indices[] = { 0, 1, 2, 2, 1, 3 };
	static RwIm2DVertex arrowVerts[4];

	RwCamera *cam = RwCameraGetCurrentCamera();
	float recipz = 1.0f/RwCameraGetNearClipPlane(cam);

	int width = RwRasterGetWidth(arrow);
	int height = RwRasterGetHeight(arrow);

	int left = r.x + (r.w - width)/2;
	int right = left + width;
	int top = r.y;
	int bottom = r.y+r.h;

	float umin = HALFPX / width;
	float vmin = HALFPX / height;
	float umax = (width + HALFPX) / width;
	float vmax = (height + HALFPX) / height;
	if(direction < 0){
		vmin = (height - HALFPX) / height;
		vmax = -HALFPX / height;
	}

	if(style){
		RwIm2DVertexSetScreenX(&arrowVerts[0], r.x);
		RwIm2DVertexSetScreenY(&arrowVerts[0], r.y-1);
		RwIm2DVertexSetScreenZ(&arrowVerts[0], RwIm2DGetNearScreenZ());
		RwIm2DVertexSetCameraZ(&arrowVerts[0], RwCameraGetNearClipPlane(cam));
		RwIm2DVertexSetRecipCameraZ(&arrowVerts[0], recipz);
		RwIm2DVertexSetIntRGBA(&arrowVerts[0], 132, 132, 132, 255);

		RwIm2DVertexSetScreenX(&arrowVerts[1], r.x+r.w);
		RwIm2DVertexSetScreenY(&arrowVerts[1], r.y-1);
		RwIm2DVertexSetScreenZ(&arrowVerts[1], RwIm2DGetNearScreenZ());
		RwIm2DVertexSetCameraZ(&arrowVerts[1], RwCameraGetNearClipPlane(cam));
		RwIm2DVertexSetRecipCameraZ(&arrowVerts[1], recipz);
		RwIm2DVertexSetIntRGBA(&arrowVerts[1], 132, 132, 132, 255);

		RwIm2DVertexSetScreenX(&arrowVerts[2], r.x);
		RwIm2DVertexSetScreenY(&arrowVerts[2], r.y+r.h+1);
		RwIm2DVertexSetScreenZ(&arrowVerts[2], RwIm2DGetNearScreenZ());
		RwIm2DVertexSetCameraZ(&arrowVerts[2], RwCameraGetNearClipPlane(cam));
		RwIm2DVertexSetRecipCameraZ(&arrowVerts[2], recipz);
		RwIm2DVertexSetIntRGBA(&arrowVerts[2], 132, 132, 132, 255);

		RwIm2DVertexSetScreenX(&arrowVerts[3], r.x+r.w);
		RwIm2DVertexSetScreenY(&arrowVerts[3], r.y+r.h+1);
		RwIm2DVertexSetScreenZ(&arrowVerts[3], RwIm2DGetNearScreenZ());
		RwIm2DVertexSetCameraZ(&arrowVerts[3], RwCameraGetNearClipPlane(cam));
		RwIm2DVertexSetRecipCameraZ(&arrowVerts[3], recipz);
		RwIm2DVertexSetIntRGBA(&arrowVerts[3], 132, 132, 132, 255);

		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
		RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
		RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, arrowVerts, 4, indices, 6);
	}


	RwIm2DVertexSetScreenX(&arrowVerts[0], left);
	RwIm2DVertexSetScreenY(&arrowVerts[0], top);
	RwIm2DVertexSetScreenZ(&arrowVerts[0], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&arrowVerts[0], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&arrowVerts[0], recipz);
	RwIm2DVertexSetIntRGBA(&arrowVerts[0], 255, 255, 255, 255);
	RwIm2DVertexSetU(&arrowVerts[0], umin, recipz);
	RwIm2DVertexSetV(&arrowVerts[0], vmin, recipz);

	RwIm2DVertexSetScreenX(&arrowVerts[1], right);
	RwIm2DVertexSetScreenY(&arrowVerts[1], top);
	RwIm2DVertexSetScreenZ(&arrowVerts[1], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&arrowVerts[1], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&arrowVerts[1], recipz);
	RwIm2DVertexSetIntRGBA(&arrowVerts[1], 255, 255, 255, 255);
	RwIm2DVertexSetU(&arrowVerts[1], umax, recipz);
	RwIm2DVertexSetV(&arrowVerts[1], vmin, recipz);

	RwIm2DVertexSetScreenX(&arrowVerts[2], left);
	RwIm2DVertexSetScreenY(&arrowVerts[2], bottom);
	RwIm2DVertexSetScreenZ(&arrowVerts[2], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&arrowVerts[2], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&arrowVerts[2], recipz);
	RwIm2DVertexSetIntRGBA(&arrowVerts[2], 255, 255, 255, 255);
	RwIm2DVertexSetU(&arrowVerts[2], umin, recipz);
	RwIm2DVertexSetV(&arrowVerts[2], vmax, recipz);

	RwIm2DVertexSetScreenX(&arrowVerts[3], right);
	RwIm2DVertexSetScreenY(&arrowVerts[3], bottom);
	RwIm2DVertexSetScreenZ(&arrowVerts[3], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&arrowVerts[3], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&arrowVerts[3], recipz);
	RwIm2DVertexSetIntRGBA(&arrowVerts[3], 255, 255, 255, 255);
	RwIm2DVertexSetU(&arrowVerts[3], umax, recipz);
	RwIm2DVertexSetV(&arrowVerts[3], vmax, recipz);

	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, arrow);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, arrowVerts, 4, indices, 6);
}





/*
 * Generate interfaces
 */


#define X(NAME, TYPE, unused1, unused2) \
MenuEntry*																			\
DebugMenuAdd##NAME(const char *path, const char *name, TYPE *ptr, TriggerFunc triggerFunc, TYPE step, TYPE lowerBound, TYPE upperBound, const char **strings)	\
{																				\
	Menu *m = findMenu(path);																\
	if(m == nil)																		\
		return nil;																		\
	MenuEntry *e = new MenuEntry_##NAME(name, ptr, triggerFunc, step, lowerBound, upperBound, strings);							\
	m->appendEntry(e);							\
	return e;																		\
}
INTTYPES
#undef X

#define X(NAME, TYPE, unused1, unused2) \
MenuEntry*																			\
DebugMenuAdd##NAME(const char *path, const char *name, TYPE *ptr, TriggerFunc triggerFunc, TYPE step, TYPE lowerBound, TYPE upperBound)	\
{																				\
	Menu *m = findMenu(path);																\
	if(m == nil)																		\
		return nil;																		\
	MenuEntry *e = new MenuEntry_##NAME(name, ptr, triggerFunc, step, lowerBound, upperBound);								\
	m->appendEntry(e);							\
	return e;																		\
}
FLOATTYPES
#undef X

MenuEntry*																			\
DebugMenuAddCmd(const char *path, const char *name, TriggerFunc triggerFunc)
{
	Menu *m = findMenu(path);
	if(m == nil)
		return nil;
	MenuEntry *e = new MenuEntry_Cmd(name, triggerFunc);
	m->appendEntry(e);
	return e;
}

void
DebugMenuEntrySetWrap(MenuEntry *e, bool wrap)
{
	if(e && e->type == MENUVAR)
		((MenuEntry_Var*)e)->wrapAround = wrap;
}

void
DebugMenuEntrySetStrings(MenuEntry *e, const char **strings)
{
	if(e && e->type == MENUVAR_INT)
		((MenuEntry_Int*)e)->setStrings(strings);
}

void
DebugMenuEntrySetAddress(MenuEntry *e, void *addr)
{
	if(e && e->type == MENUVAR){
		MenuEntry_Var *ev = (MenuEntry_Var*)e;
		// HACK - we know the variable field is at the same address
		// for all int/float classes. let's hope it stays that way.
		if(ev->vartype = MENUVAR_INT)
			((MenuEntry_Int32*)e)->variable = (int32*)addr;
		else if(ev->vartype = MENUVAR_FLOAT)
			((MenuEntry_Float32*)e)->variable = (float*)addr;
	}
}
#endif