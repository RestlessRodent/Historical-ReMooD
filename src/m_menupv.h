
/****************
*** CONSTANTS ***
****************/

/* M_SWidFlags_t -- Widget flags */
typedef enum M_SWidFlags_e
{
	MSWF_NOSELECT		= UINT32_C(0x00000001),	// Cannot be selected
	MSWF_NOHANDLEEVT	= UINT32_C(0x00000002),	// Do not handle events
	MSWF_DISABLED		= UINT32_C(0x00000004),	// Disabled
} M_SWidFlags_t;

/* M_CreateGameOpts_e -- Create game options */
enum M_CreateGameOpts_e
{
	MCGO_NULL,
	
	MCGO_IWAD,
	
	NUMMCGO
};

/*****************
*** STRUCTURES ***
*****************/

typedef struct M_SWidget_s M_SWidget_t;

/* M_SWidget_t -- Simple menu widget */
struct M_SWidget_s
{
	uint32_t Flags;								// Widget Flags
	int32_t Screen;								// Screen
	
	int32_t x, y;								// X/Y position
	int32_t w, h;								// Width/Height
	int32_t offx, offy;							// X/Y offset
	
	int32_t dx, dy;								// Draw X/Y
	int32_t dw, dh;								// Draw W/H
	
	M_SWidget_t* Parent;						// Parent Widget
	
	M_SWidget_t** Kids;							// Kid Widgets
	int32_t NumKids;							// Number of kids
	
	int32_t CursorOn;							// Curson on which kid?
	M_SMMenus_t SubMenu;						// SubMenu to open
	int32_t Option;								// Option
	D_Prof_t* Prof;								// Profile to modify
	
	// Drawing
	void (*DCursor)(M_SWidget_t* const, M_SWidget_t* const);
	void (*DDraw)(M_SWidget_t* const);
	
	// Functions
	void (*FDestroy)(M_SWidget_t* const);
	bool_t (*FEvent)(M_SWidget_t* const, const I_EventEx_t* const);
	bool_t (*FLeftRight)(M_SWidget_t* const, const int32_t);
	bool_t (*FUpDown)(M_SWidget_t* const, const int32_t);
	bool_t (*FSelect)(M_SWidget_t* const);
	bool_t (*FCancel)(M_SWidget_t* const);
	void (*FTicker)(M_SWidget_t* const);
	
	union
	{
		struct
		{
			V_Image_t* Pic;						// Picture to draw
		} Image;								// Simple Image
		
		struct
		{
			V_Image_t* Skulls[2];				// Skulls
		} MainMenu;								// Main Menu Stuff
		
		struct
		{
			VideoFont_t Font;					// Font to draw in
			uint32_t Flags;						// Draw flags
			const char** Ref;					// Reference to text
			const char** ValRef;				// Value Reference
			int32_t Pivot;						// Current pivot location
			CONL_StaticVar_t* CVar;				// Console Variables
			CONL_VarPossibleValue_t* Possible;	// Possible Values
			P_XGSVariable_t* Var;				// Variable
			P_XGSBitID_t NextVar;				// Next Variable
			uint32_t ValFlags;					// Value Draw Flags
		} Label;
		
		struct
		{
			VideoFont_t Font;					// Font to draw in
			uint32_t Flags;						// Draw flags
			CONCTI_Inputter_t* Inputter;		// Text Inputter
			bool_t StealInput;					// Input Stolen
			bool_t OldSteal;					// Old Steal Input
		} TextBox;
	} Data;										// Widget Data
};

/**************
*** GLOBALS ***
**************/

extern D_Prof_t* g_DoProf;

/****************
*** FUNCTIONS ***
****************/

void M_StackPop(const int32_t a_ScreenID);
void M_StackPopAllScreen(const int32_t a_Screen);
void M_StackPopAll(void);

int32_t M_HelpInitIWADList(CONL_VarPossibleValue_t** const a_PossibleOut);

void M_MainMenu_DCursor(M_SWidget_t* const a_Widget, M_SWidget_t* const a_Sub);
bool_t M_SubMenu_FSelect(M_SWidget_t* const a_Widget);
bool_t M_NewGameClassic_FSelect(M_SWidget_t* const a_Widget);
bool_t M_NewGameEpi_FSelect(M_SWidget_t* const a_Widget);
bool_t M_NewGameSkill_FSelect(M_SWidget_t* const a_Widget);
bool_t M_QuitGame_DisconFSelect(M_SWidget_t* const a_Widget);
bool_t M_QuitGame_StopWatchFSelect(M_SWidget_t* const a_Widget);
bool_t M_QuitGame_StopRecordFSelect(M_SWidget_t* const a_Widget);
bool_t M_QuitGame_LogOffFSelect(M_SWidget_t* const a_Widget);
bool_t M_QuitGame_ExitFSelect(M_SWidget_t* const a_Widget);
void M_QuitGame_FTicker(M_SWidget_t* const a_Widget);
void M_ACG_CreateFSelect(M_SWidget_t* const a_Widget);
bool_t M_CTUS_BoxCallBack(struct CONCTI_Inputter_s*, const char* const);
void M_CTUS_ConnectFSelect(M_SWidget_t* const a_Widget);

void M_ProfMan_FTicker(M_SWidget_t* const a_Widget);
bool_t M_ProfMan_CreateProf(M_SWidget_t* const a_Widget);
bool_t M_ProfMan_IndvFSel(M_SWidget_t* const a_Widget);
bool_t M_ProfMan_AcctBCB(struct CONCTI_Inputter_s*, const char* const);

