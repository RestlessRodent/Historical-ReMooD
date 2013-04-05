
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

/* M_SWidget_t -- Simple menu widget */
typedef struct M_SWidget_s
{
	uint32_t Flags;								// Widget Flags
	int32_t Screen;								// Screen
	
	int32_t x, y;								// X/Y position
	int32_t w, h;								// Width/Height
	int32_t offx, offy;							// X/Y offset
	
	int32_t dx, dy;								// Draw X/Y
	int32_t dw, dh;								// Draw W/H
	
	struct M_SWidget_s* Parent;					// Parent Widget
	
	struct M_SWidget_s** Kids;					// Kid Widgets
	int32_t NumKids;							// Number of kids
	
	int32_t CursorOn;							// Curson on which kid?
	M_SMMenus_t SubMenu;						// SubMenu to open
	int32_t Option;								// Option
	
	// Drawing
	void (*DCursor)(struct M_SWidget_s* const, struct M_SWidget_s* const);
	void (*DDraw)(struct M_SWidget_s* const);
	
	// Functions
	void (*FDestroy)(struct M_SWidget_s* const);
	bool_t (*FEvent)(struct M_SWidget_s* const, const I_EventEx_t* const);
	bool_t (*FLeftRight)(struct M_SWidget_s* const, const int32_t);
	bool_t (*FUpDown)(struct M_SWidget_s* const, const int32_t);
	bool_t (*FSelect)(struct M_SWidget_s* const);
	bool_t (*FCancel)(struct M_SWidget_s* const);
	void (*FTicker)(struct M_SWidget_s* const);
	
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
} M_SWidget_t;

/****************
*** FUNCTIONS ***
****************/

void M_StackPop(const int32_t a_ScreenID);
void M_StackPopAllScreen(const int32_t a_Screen);
void M_StackPopAll(void);

int32_t M_HelpInitIWADList(CONL_VarPossibleValue_t** const a_PossibleOut);

void M_MainMenu_DCursor(struct M_SWidget_s* const a_Widget, struct M_SWidget_s* const a_Sub);
bool_t M_SubMenu_FSelect(struct M_SWidget_s* const a_Widget);
bool_t M_NewGameClassic_FSelect(struct M_SWidget_s* const a_Widget);
bool_t M_NewGameEpi_FSelect(struct M_SWidget_s* const a_Widget);
bool_t M_NewGameSkill_FSelect(struct M_SWidget_s* const a_Widget);
bool_t M_QuitGame_DisconFSelect(struct M_SWidget_s* const a_Widget);
bool_t M_QuitGame_StopWatchFSelect(struct M_SWidget_s* const a_Widget);
bool_t M_QuitGame_StopRecordFSelect(struct M_SWidget_s* const a_Widget);
bool_t M_QuitGame_LogOffFSelect(struct M_SWidget_s* const a_Widget);
bool_t M_QuitGame_ExitFSelect(struct M_SWidget_s* const a_Widget);
void M_QuitGame_FTicker(struct M_SWidget_s* const a_Widget);
void M_ACG_CreateFSelect(struct M_SWidget_s* const a_Widget);
bool_t M_CTUS_BoxCallBack(struct CONCTI_Inputter_s*, const char* const);
void M_CTUS_ConnectFSelect(struct M_SWidget_s* const a_Widget);

