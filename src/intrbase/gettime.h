#if defined(__unix__)

/* I_GetTimeMS() -- Returns time since the game started (in MS) */
uint32_t I_GetTimeMS(void)
{
	struct timeval TV;
	static uint32_t BaseTime;
	uint32_t NowTime;
	
	/* Get the current time */
	gettimeofday(&TV, NULL);
	
	/* Obtain the current time */
	NowTime = (TV.tv_sec * UINT32_C(1000)) + (TV.tv_usec / UINT32_C(1000));
	
	// No last time?
	if (!BaseTime)
		BaseTime = NowTime;
	
	/* Return difference */
	return NowTime - BaseTime;
}

#elif _POSIX_C_SOURCE >= 199309L

/* I_GetTimeMS() -- Returns time since the game started (in MS) */
uint32_t I_GetTimeMS(void)
{
	struct timespec Spec;
	static uint32_t BaseTime;
	uint32_t NowTime;
	
	/* Get the current monotonic time */
	clock_gettime(CLOCK_MONOTONIC, &Spec);
	
	/* Obtain the current time */
	NowTime = (Spec.tv_sec * UINT32_C(1000)) + (Spec.tv_nsec / UINT32_C(1000000));
	
	// No last time?
	if (!BaseTime)
		BaseTime = NowTime;
	
	/* Return difference */
	return NowTime - BaseTime;
}

#elif defined(_WIN32)

/* I_GetTimeMS() -- Returns time since the game started (in MS) */
uint32_t I_GetTimeMS(void)
{
	static DWORD basetime = 0;
	DWORD ticks = 0;
	
	ticks = GetTickCount();

	if (!basetime)
		basetime = ticks;

	return (uint32_t)(ticks - basetime);
}

#else

/* I_GetTimeMS() -- Returns time since the game started (in MS) */
uint32_t I_GetTimeMS(void)
{
	static clock_t FirstClock;
	clock_t ThisClock = 0;
	
	/* Get current clock */
	ThisClock = clock();
	
	// FirstClock not set?
	if (!FirstClock)
		FirstClock = ThisClock;
		
	/* Return time passed */
	return ((ThisClock - FirstClock) * 1000) / CLOCKS_PER_SEC;
}

#endif

