//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The downtrodden citizens of City 17. Timid when unarmed, they will
//			rise up against their Combine oppressors when given a weapon.
//
//=============================================================================//

#ifndef	NPC_CITIZEN_H
#define	NPC_CITIZEN_H

#include "npc_playercompanion.h"

#include "ai_behavior_functank.h"
#ifdef MAPBASE
#include "ai_behavior_rappel.h"
#include "ai_behavior_police.h"
#endif
#ifdef EZ2
#include "ez2/ai_behavior_surrender.h"
#endif

struct SquadCandidate_t;

//-----------------------------------------------------------------------------
//
// CLASS: CNPC_Citizen
//
//-----------------------------------------------------------------------------

//-------------------------------------
// Spawnflags
//-------------------------------------

#define SF_CITIZEN_FOLLOW			( 1 << 16 )	//65536 follow the player as soon as I spawn.
#define	SF_CITIZEN_MEDIC			( 1 << 17 )	//131072
#define SF_CITIZEN_RANDOM_HEAD		( 1 << 18 )	//262144
#define SF_CITIZEN_AMMORESUPPLIER	( 1 << 19 )	//524288
#define SF_CITIZEN_NOT_COMMANDABLE	( 1 << 20 ) //1048576
#define SF_CITIZEN_IGNORE_SEMAPHORE ( 1 << 21 ) //2097152		Work outside the speech semaphore system
#define SF_CITIZEN_RANDOM_HEAD_MALE	( 1 << 22 )	//4194304
#define SF_CITIZEN_RANDOM_HEAD_FEMALE ( 1 << 23 )//8388608
#define SF_CITIZEN_USE_RENDER_BOUNDS ( 1 << 24 )//16777216
#ifdef MAPBASE
#define SF_CITIZEN_PLAYER_TOGGLE_SQUAD ( 1 << 25 ) //33554432		Prevents the citizen from joining the squad automatically, but still being commandable if the player toggles it
#endif

//-------------------------------------
// Animation events
//-------------------------------------

enum CitizenType_t
{
	CT_DEFAULT,
	CT_DOWNTRODDEN,
	CT_REFUGEE,
	CT_REBEL,
	CT_UNIQUE,
	CT_BRUTE,
	CT_LONGFALL,
	CT_ARCTIC,
	CT_ARBEIT,		// Pre-war Arbeit employees
	CT_ARBEIT_SEC,	// Pre-war Arbeit security guards
};

//-----------------------------------------------------------------------------
// Citizen expression types
//-----------------------------------------------------------------------------
enum CitizenExpressionTypes_t
{
	CIT_EXP_UNASSIGNED,	// Defaults to this, selects other in spawn.

	CIT_EXP_SCARED,
	CIT_EXP_NORMAL,
	CIT_EXP_ANGRY,

	CIT_EXP_LAST_TYPE,
};

//-------------------------------------

class CNPC_Citizen : public CNPC_PlayerCompanion
{
	DECLARE_CLASS( CNPC_Citizen, CNPC_PlayerCompanion );
public:
	CNPC_Citizen()
	 :	m_iHead( -1 )
#ifdef EZ
		, m_iWillpowerModifier( 0 )
#endif
	{
#ifdef EZ2
		m_bInvestigateSounds = true; // Rebels should investigate sounds
#endif
	}

	//---------------------------------
	bool			CreateBehaviors();
	void			Precache();
	void			PrecacheAllOfType( CitizenType_t );
	void			Spawn();
	void			PostNPCInit();
	virtual void	SelectModel();
	void			SelectExpressionType();
	void			Activate();
	virtual void	OnGivenWeapon( CBaseCombatWeapon *pNewWeapon );
	void			FixupMattWeapon();

#ifdef HL2_EPISODIC
	virtual float	GetJumpGravity() const		{ return 1.8f; }
#endif//HL2_EPISODIC

	void			OnRestore();
	
	//---------------------------------
	string_t 		GetModelName() const;
	
	Class_T 		Classify();

	bool 			ShouldAlwaysThink();

	//---------------------------------
	// Behavior
	//---------------------------------
	bool			ShouldBehaviorSelectSchedule( CAI_BehaviorBase *pBehavior );
	void 			GatherConditions();
	void			PredictPlayerPush();
	void 			PrescheduleThink();
	void			BuildScheduleTestBits();

	bool			FInViewCone( CBaseEntity *pEntity );

	int				SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );
	int				SelectSchedule();

	int 			SelectSchedulePriorityAction();
	int 			SelectScheduleHeal();
	int 			SelectScheduleRetrieveItem();
	int 			SelectScheduleNonCombat();
	int 			SelectScheduleManhackCombat();
	int 			SelectScheduleCombat();
	bool			ShouldDeferToFollowBehavior();
	int 			TranslateSchedule( int scheduleType );


	bool			ShouldAcceptGoal( CAI_BehaviorBase *pBehavior, CAI_GoalEntity *pGoal );
	void			OnClearGoal( CAI_BehaviorBase *pBehavior, CAI_GoalEntity *pGoal );
	
	void 			StartTask( const Task_t *pTask );
	void 			RunTask( const Task_t *pTask );
	
	Activity		NPC_TranslateActivity( Activity eNewActivity );
#ifdef EZ
	void			TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
	void			Event_Killed( const CTakeDamageInfo &info );
	void			OnChangeActivity( Activity eNewActivity );
	void			Event_KilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info );
#endif
	void 			HandleAnimEvent( animevent_t *pEvent );
	void			TaskFail( AI_TaskFailureCode_t code );

#ifndef MAPBASE // Moved to CAI_BaseNPC
	void 			PickupItem( CBaseEntity *pItem );
#endif

	void 			SimpleUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	bool			IgnorePlayerPushing( void );

	int				DrawDebugTextOverlays( void );

	virtual const char *SelectRandomExpressionForState( NPC_STATE state );

#ifdef EZ
	// Blixibon - Lets citizens ignite from gas cans, etc.
	bool			AllowedToIgnite( void ) { return true; }

	// 1upD - Citizens can gib if killed by massive explosive damage or acid
	virtual bool		ShouldGib( const CTakeDamageInfo &info );
	virtual bool		CorpseGib( const CTakeDamageInfo &info );

	virtual CSprite		* GetGlowSpritePtr( int i );
	virtual void		  SetGlowSpritePtr( int i, CSprite * sprite );
	virtual EyeGlow_t	* GetEyeGlowData( int i );
	virtual int			  GetNumGlows();
#endif

	//---------------------------------
	// Combat
	//---------------------------------
	bool 			OnBeginMoveAndShoot();
	void 			OnEndMoveAndShoot();
#ifndef EZ
	virtual bool	UseAttackSquadSlots()	{ return false; }
#else
	virtual bool	HasAttackSlot();
#endif

	void 			LocateEnemySound();

	bool			IsManhackMeleeCombatant();
	
	Vector 			GetActualShootPosition( const Vector &shootOrigin );
	void 			OnChangeActiveWeapon( CBaseCombatWeapon *pOldWeapon, CBaseCombatWeapon *pNewWeapon );

	bool			ShouldLookForBetterWeapon();
#ifdef EZ
	WeaponProficiency_t CalcWeaponProficiency(CBaseCombatWeapon *pWeapon); // Added by 1upD - Citizen proficiency should be configurable
	bool			IsJumpLegal(const Vector & startPos, const Vector & apex, const Vector & endPos) const; // Added by 1upD - Override for jump rebels
	bool			TestShootPosition( const Vector &vecShootPos, const Vector &targetPos );

	//---------------------------------
	// Willpower
	//---------------------------------
	void			GatherWillpowerConditions();
	int				MeleeAttack1Conditions( float flDot, float flDist );

	Disposition_t	IRelationType(CBaseEntity *pTarget);
#ifdef EZ2
	bool			JustStartedFearing( CBaseEntity *pTarget ); // Blixibon - Needed so the player's speech AI doesn't pick this up as D_FR before it's apparent (e.g. fast, rapid kills)

	CitizenType_t	GetCitiznType() { return m_Type; }

	bool			GiveBackupWeapon( CBaseCombatWeapon * pWeapon, CBaseEntity * pActivator );
	bool			TrySpeakBeg();

	inline bool		IsSurrendered() { return m_SurrenderBehavior.IsSurrendered(); } //{ return GetContextValue( "surrendered" )[0] == '1'; };
	inline bool		IsSurrenderIdle() { return m_SurrenderBehavior.IsSurrenderIdle(); }
	inline bool		CanSurrender() { return m_SurrenderBehavior.CanSurrender(); }
	inline bool		SurrenderAutomatically() { return m_SurrenderBehavior.SurrenderAutomatically(); }
#endif
	void			MsgWillpower(const tchar* pMsg, int willpower);
	int 			TranslateWillpowerSchedule(int scheduleType);
	int				TranslateSuppressingFireSchedule(int scheduleType);
	int				SelectRangeAttack2Schedule();
	void			OnStartSchedule( int scheduleType );
	bool			FindDecoyObject(void);
	bool			FindEnemyCoverTarget(void);
	void			AimGun();

	const char*		GetSquadSlotDebugName(int iSquadSlot); // Debug names for new squad slots

	float			m_flLastWillpowerMsgTime;
	Vector			m_vecDecoyObjectTarget;
#endif
	//---------------------------------
	// Damage handling
	//---------------------------------
	int 			OnTakeDamage_Alive( const CTakeDamageInfo &info );
#ifdef EZ
	float			GetHitgroupDamageMultiplier( int iHitGroup, const CTakeDamageInfo &info );
#endif

#ifdef MAPBASE
	//---------------------------------
	void			ModifyOrAppendCriteria( AI_CriteriaSet& set );
#endif

#ifdef EZ2
	// Blixibon - Gets criteria the player should use in speech
	virtual void		ModifyOrAppendCriteriaForPlayer( CBasePlayer *pPlayer, AI_CriteriaSet& set );

	bool			GetGameTextSpeechParams( hudtextparms_t &params );
#endif
	
	//---------------------------------
	// Commander mode
	//---------------------------------
	bool 			IsCommandable();
	bool			IsPlayerAlly( CBasePlayer *pPlayer = NULL );
	bool			CanJoinPlayerSquad();
	bool			WasInPlayerSquad();
	bool			HaveCommandGoal() const;
	bool			IsCommandMoving();
	bool			ShouldAutoSummon();
	bool 			IsValidCommandTarget( CBaseEntity *pTarget );
	bool 			NearCommandGoal();
	bool 			VeryFarFromCommandGoal();
	bool 			TargetOrder( CBaseEntity *pTarget, CAI_BaseNPC **Allies, int numAllies );
	void 			MoveOrder( const Vector &vecDest, CAI_BaseNPC **Allies, int numAllies );
	void			OnMoveOrder();
	void 			CommanderUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
#ifdef MAPBASE
	bool			ShouldAllowSquadToggleUse( CBasePlayer *pPlayer );
#endif
	bool			ShouldSpeakRadio( CBaseEntity *pListener );
	void			OnMoveToCommandGoalFailed();
	void			AddToPlayerSquad();
	void			RemoveFromPlayerSquad();
	void 			TogglePlayerSquadState();
	void			UpdatePlayerSquad();
	static int __cdecl PlayerSquadCandidateSortFunc( const SquadCandidate_t *, const SquadCandidate_t * );
	void 			FixupPlayerSquad();
	void 			ClearFollowTarget();
	void 			UpdateFollowCommandPoint();
	bool			IsFollowingCommandPoint();
	CAI_BaseNPC *	GetSquadCommandRepresentative();
	void			SetSquad( CAI_Squad *pSquad );
	void			AddInsignia();
	void			RemoveInsignia();
	bool			SpeakCommandResponse( AIConcept_t concept, const char *modifiers = NULL );

#ifdef MAPBASE
	virtual void	SetPlayerAvoidState( void );
#endif
	
	//---------------------------------
	// Scanner interaction
	//---------------------------------
	float 			GetNextScannerInspectTime() { return m_fNextInspectTime; }
	void			SetNextScannerInspectTime( float flTime ) { m_fNextInspectTime = flTime; }
	bool			HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);
	
	//---------------------------------
	// Hints
	//---------------------------------
	bool			FValidateHintType ( CAI_Hint *pHint );

	//---------------------------------
	// Special abilities
	//---------------------------------
	bool 			IsMedic() 			{ return HasSpawnFlags(SF_CITIZEN_MEDIC); }
	bool 			IsAmmoResupplier() 	{ return HasSpawnFlags(SF_CITIZEN_AMMORESUPPLIER); }
	
	bool 			CanHeal();
	bool 			ShouldHealTarget( CBaseEntity *pTarget, bool bActiveUse = false );
#if HL2_EPISODIC
	bool 			ShouldHealTossTarget( CBaseEntity *pTarget, bool bActiveUse = false );
#endif
	void 			Heal();

	bool			ShouldLookForHealthItem();

#if HL2_EPISODIC
	void			TossHealthKit( CBaseCombatCharacter *pThrowAt, const Vector &offset ); // create a healthkit and throw it at someone
	void			InputForceHealthKitToss( inputdata_t &inputdata );
#endif
	
	//---------------------------------
	// Inputs
	//---------------------------------
	void			InputRemoveFromPlayerSquad( inputdata_t &inputdata ) { RemoveFromPlayerSquad(); }
	void 			InputStartPatrolling( inputdata_t &inputdata );
	void 			InputStopPatrolling( inputdata_t &inputdata );
	void			InputSetCommandable( inputdata_t &inputdata );
#ifdef MAPBASE
	void			InputSetUnCommandable( inputdata_t &inputdata );
#endif
	void			InputSetMedicOn( inputdata_t &inputdata );
	void			InputSetMedicOff( inputdata_t &inputdata );
	void			InputSetAmmoResupplierOn( inputdata_t &inputdata );
	void			InputSetAmmoResupplierOff( inputdata_t &inputdata );
	void			InputSpeakIdleResponse( inputdata_t &inputdata );
#ifdef MAPBASE
	void			InputSetPoliceGoal( inputdata_t &inputdata );
#endif
#ifdef EZ2
	void			InputSurrender( inputdata_t &inputdata );
	void			InputSetSurrenderFlags( inputdata_t &inputdata );
	void			InputAddSurrenderFlags( inputdata_t &inputdata );
	void			InputRemoveSurrenderFlags( inputdata_t &inputdata );
	void			InputSetWillpowerModifier( inputdata_t &inputdata );
	void			InputSetWillpowerDisabled( inputdata_t &inputdata );
	void			InputSetSuppressiveFireDisabled( inputdata_t &inputdata );
	void			InputForcePanic( inputdata_t &inputdata );
#endif

	//---------------------------------
	//	Sounds & speech
	//---------------------------------
	void			FearSound( void );
	void			DeathSound( const CTakeDamageInfo &info );
	bool			UseSemaphore( void );

	virtual void	OnChangeRunningBehavior( CAI_BehaviorBase *pOldBehavior,  CAI_BehaviorBase *pNewBehavior );

#ifdef MAPBASE
	int				GetCitizenType() { return (int)m_Type; }
	void			SetCitizenType( int iType ) { m_Type = (CitizenType_t)iType; }
#endif

private:
	//-----------------------------------------------------
	// Conditions, Schedules, Tasks
	//-----------------------------------------------------
	enum
	{
		COND_CIT_PLAYERHEALREQUEST = BaseClass::NEXT_CONDITION,
		COND_CIT_COMMANDHEAL,
		COND_CIT_HURTBYFIRE,
		COND_CIT_START_INSPECTION,
#ifdef EZ
		COND_CIT_WILLPOWER_VERY_LOW,
		COND_CIT_WILLPOWER_LOW,
		COND_CIT_WILLPOWER_HIGH,
		COND_CIT_ON_FIRE,
		COND_CIT_DISARMED,
		NEXT_CONDITION,
#endif
		
		SCHED_CITIZEN_PLAY_INSPECT_ACTIVITY = BaseClass::NEXT_SCHEDULE,
		SCHED_CITIZEN_HEAL,
		SCHED_CITIZEN_RANGE_ATTACK1_RPG,
		SCHED_CITIZEN_PATROL,
		SCHED_CITIZEN_MOURN_PLAYER,
		SCHED_CITIZEN_SIT_ON_TRAIN,
		SCHED_CITIZEN_STRIDER_RANGE_ATTACK1_RPG,
#ifdef HL2_EPISODIC
		SCHED_CITIZEN_HEAL_TOSS,
#endif
#ifdef EZ
		SCHED_CITIZEN_RANGE_ATTACK1_ADVANCE,
		SCHED_CITIZEN_RANGE_ATTACK1_SUPPRESS,
		SCHED_CITIZEN_BURNING_STAND,
		NEXT_SCHEDULE,
#endif
		
		TASK_CIT_HEAL = BaseClass::NEXT_TASK,
		TASK_CIT_RPG_AUGER,
		TASK_CIT_PLAY_INSPECT_SEQUENCE,
		TASK_CIT_SIT_ON_TRAIN,
		TASK_CIT_LEAVE_TRAIN,
		TASK_CIT_SPEAK_MOURNING,
#ifdef HL2_EPISODIC
		TASK_CIT_HEAL_TOSS,
#endif
#ifdef EZ
		TASK_CIT_DIE_INSTANTLY,
		TASK_CIT_PAINT_SUPPRESSION_TARGET,
		NEXT_TASK,
#endif
	};

	//-----------------------------------------------------
	
	int				m_nInspectActivity;
	float			m_flNextFearSoundTime;
	float			m_flStopManhackFlinch;
	float			m_fNextInspectTime;		// Next time I'm allowed to get inspected by a scanner
	float			m_flPlayerHealTime;
	float			m_flNextHealthSearchTime; // Next time I'm allowed to look for a healthkit
	float			m_flAllyHealTime;
	float			m_flPlayerGiveAmmoTime;
	string_t		m_iszAmmoSupply;
	int				m_iAmmoAmount;
	bool			m_bRPGAvoidPlayer;
	bool			m_bShouldPatrol;
	string_t		m_iszOriginalSquad;
	float			m_flTimeJoinedPlayerSquad;
	bool			m_bWasInPlayerSquad;
	float			m_flTimeLastCloseToPlayer;
	string_t		m_iszDenyCommandConcept;
#ifdef MAPBASE
	bool			m_bTossesMedkits;
	bool			m_bAlternateAiming;
#endif

	CSimpleSimTimer	m_AutoSummonTimer;
	Vector			m_vAutoSummonAnchor;

	CitizenType_t	m_Type;
	CitizenExpressionTypes_t	m_ExpressionType;

	int				m_iHead;

	static CSimpleSimTimer gm_PlayerSquadEvaluateTimer;

	float			m_flTimePlayerStare;	// The game time at which the player started staring at me.
	float			m_flTimeNextHealStare;	// Next time I'm allowed to heal a player who is staring at me.
#ifdef EZ
	int				m_iWillpowerModifier;	// 1upD - Amount of 'mental fortitude' points before panic
	bool			m_bWillpowerDisabled;	// 1upD - Override willpower behavior
	bool			m_bSuppressiveFireDisabled; // 1upD - Override suppressive fire behavior

	bool			m_bUsedBackupWeapon;	// 1upD - Has this rebel been given a backup weapon already?

	CSprite			*m_pShoulderGlow;
#endif
	//-----------------------------------------------------
	//	Outputs
	//-----------------------------------------------------
	COutputEvent		m_OnJoinedPlayerSquad;
	COutputEvent		m_OnLeftPlayerSquad;
	COutputEvent		m_OnFollowOrder;
	COutputEvent		m_OnStationOrder; 
	COutputEvent		m_OnPlayerUse;
	COutputEvent		m_OnNavFailBlocked;
#ifdef MAPBASE
	COutputEvent		m_OnHealedNPC;
	COutputEvent		m_OnHealedPlayer;
	COutputEHANDLE		m_OnThrowMedkit;
	COutputEvent		m_OnGiveAmmo;
#endif

#ifdef EZ2
	COutputEvent		m_OnSurrender;
	COutputEvent		m_OnStopSurrendering;
#endif

	//-----------------------------------------------------
#ifdef MAPBASE
	CAI_RappelBehavior		m_RappelBehavior;
	CAI_PolicingBehavior	m_PolicingBehavior;

	// Rappel
	virtual bool IsWaitingToRappel( void ) { return m_RappelBehavior.IsWaitingToRappel(); }
	void BeginRappel() { m_RappelBehavior.BeginRappel(); }
#else // Moved to CNPC_PlayerCompanion
	CAI_FuncTankBehavior	m_FuncTankBehavior;
#endif
#ifdef EZ2
	// Used by the Arbeit helicopter
	virtual bool HasRappelBehavior() { return true; }
	virtual void StartWaitingForRappel() { m_RappelBehavior.StartWaitingForRappel(); }
#endif

#ifdef EZ2
	class CCitizenSurrenderBehavior : public CAI_SurrenderBehavior
	{
		typedef CAI_SurrenderBehavior BaseClass;

	public:
		virtual void Surrender( CBaseCombatCharacter *pCaptor );

		virtual int SelectSchedule();

		virtual void BuildScheduleTestBits();

		virtual void RunTask( const Task_t *pTask );

		virtual int ModifyResistanceValue( int iVal );

		inline CNPC_Citizen *GetOuterCit() { return static_cast<CNPC_Citizen*>(GetOuter()); }
	};

	virtual CAI_SurrenderBehavior &GetSurrenderBehavior( void ) { return m_SurrenderBehavior; }

	CCitizenSurrenderBehavior	m_SurrenderBehavior;
#endif

	CHandle<CAI_FollowGoal>	m_hSavedFollowGoalEnt;

	bool					m_bNotifyNavFailBlocked;
	bool					m_bNeverLeavePlayerSquad; // Don't leave the player squad unless killed, or removed via Entity I/O. 
	
	//-----------------------------------------------------
	
#ifdef MAPBASE_VSCRIPT
	static ScriptHook_t		g_Hook_SelectModel;
	DECLARE_ENT_SCRIPTDESC();
#endif
	DECLARE_DATADESC();
#ifdef _XBOX
protected:
#endif
	DEFINE_CUSTOM_AI;
};

//---------------------------------------------------------
//---------------------------------------------------------
inline bool CNPC_Citizen::NearCommandGoal()
{
	const float flDistSqr = COMMAND_GOAL_TOLERANCE * COMMAND_GOAL_TOLERANCE;
	return ( ( GetAbsOrigin() - GetCommandGoal() ).LengthSqr() <= flDistSqr );
}

//---------------------------------------------------------
//---------------------------------------------------------
inline bool CNPC_Citizen::VeryFarFromCommandGoal()
{
	const float flDistSqr = (12*50) * (12*50);
	return ( ( GetAbsOrigin() - GetCommandGoal() ).LengthSqr() > flDistSqr );
}



//==============================================================================
// CITIZEN PLAYER-RESPONSE SYSTEM
//
// NOTE: This system is obsolete, and left here for legacy support.
//		 It has been superseded by the ai_eventresponse system.
//
//==============================================================================
#define CITIZEN_RESPONSE_DISTANCE			768			// Maximum distance for responding citizens
#define CITIZEN_RESPONSE_REFIRE_TIME		15.0		// Time after giving a response before giving any more
#define CITIZEN_RESPONSE_GIVEUP_TIME		4.0			// Time after a response trigger was fired before discarding it without responding

enum citizenresponses_t
{
	CR_PLAYER_SHOT_GUNSHIP,		// Player has shot the gunship with a bullet weapon
	CR_PLAYER_KILLED_GUNSHIP,	// Player has destroyed the gunship
	CR_VITALNPC_DIED,			// Mapmaker specified that an NPC that was vital has died

	// Add new responses here

	MAX_CITIZEN_RESPONSES,
};

//-------------------------------------

class CCitizenResponseSystem : public CBaseEntity
{
	DECLARE_CLASS( CCitizenResponseSystem, CBaseEntity );
public:
	DECLARE_DATADESC();

	void	Spawn();
	void	OnRestore();

	void	AddResponseTrigger( citizenresponses_t	iTrigger );

	void	ResponseThink();

	//---------------------------------
	// Inputs
	//---------------------------------
	void 	InputResponseVitalNPC( inputdata_t &inputdata );

private:
	float	m_flResponseAddedTime[ MAX_CITIZEN_RESPONSES ];		// Time at which the response was added. 0 if we have no response.
	float	m_flNextResponseTime;
};

//-------------------------------------

class CSquadInsignia : public CBaseAnimating
{
	DECLARE_CLASS( CSquadInsignia, CBaseAnimating );
	void Spawn();
};

//-------------------------------------

CCitizenResponseSystem	*GetCitizenResponse();

//-----------------------------------------------------------------------------

#endif	//NPC_CITIZEN_H
