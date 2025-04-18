//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Base class for humanoid NPCs intended to fight along side player in close
// environments
//
//=============================================================================//

#ifndef NPC_PLAYERCOMPANION_H
#define NPC_PLAYERCOMPANION_H

#include "ai_playerally.h"

#include "ai_behavior_follow.h"
#include "ai_behavior_standoff.h"
#include "ai_behavior_assault.h"
#include "ai_behavior_lead.h"
#include "ai_behavior_actbusy.h"
#include "ai_behavior_fear.h"

#ifdef HL2_EPISODIC
#include "ai_behavior_operator.h"
#include "ai_behavior_passenger_companion.h"
#endif

#ifdef MAPBASE
#include "ai_behavior_functank.h"
#include "mapbase/ai_grenade.h"
#endif

#ifdef EZ2
#include "ez2/ai_behavior_surrender.h"
#endif

#if defined( _WIN32 )
#pragma once
#endif

enum AIReadiness_t
{
	AIRL_PANIC = -2,
	AIRL_STEALTH = -1,
	AIRL_RELAXED = 0,
	AIRL_STIMULATED,
	AIRL_AGITATED,
};

enum AIReadinessUse_t
{
	AIRU_NEVER,
	AIRU_ALWAYS,
	AIRU_ONLY_PLAYER_SQUADMATES,
};


class CCompanionActivityRemap : public CActivityRemap
{
public:
	CCompanionActivityRemap( void ) : 
	  m_fUsageBits( 0 ),
	  m_readiness( AIRL_RELAXED ),
	  m_bAiming( false ),
	  m_bWeaponRequired( false ),
	  m_bInVehicle( false ) {} 

	// This bitfield maps which bits of data are being utilized by this data structure, since not all criteria
	// in the parsed file are essential.  You must add corresponding bits to the definitions below and maintain
	// their state in the parsing of the file, as well as check the bitfield before accessing the data.  This
	// could be encapsulated into this class, but we'll probably move away from this model and closer to something
	// more akin to the response rules -- jdw

	int				m_fUsageBits;

	AIReadiness_t	m_readiness;
	bool			m_bAiming;
	bool			m_bWeaponRequired;
	bool			m_bInVehicle;		// For future expansion, this needs to speak more to the exact seat, role, and vehicle
};

// Usage bits for remap "extra" parsing - if these bits are set, the associated data has changed
#define bits_REMAP_READINESS		(1<<0)
#define bits_REMAP_AIMING			(1<<1)
#define bits_REMAP_WEAPON_REQUIRED	(1<<2)
#define bits_REMAP_IN_VEHICLE		(1<<3)

// Readiness modes that only change due to mapmaker scripts
#define READINESS_MIN_VALUE			-2
#define READINESS_MODE_PANIC		-2
#define READINESS_MODE_STEALTH		-1

// Readiness modes that change normally
#define READINESS_VALUE_RELAXED		0.1f
#define READINESS_VALUE_STIMULATED	0.95f
#define READINESS_VALUE_AGITATED	1.0f

class CPhysicsProp;

#ifdef MAPBASE
// If you think about it, this is really unnecessary.
//#define COMPANION_MELEE_ATTACK 1
#endif

//-----------------------------------------------------------------------------
//
// CLASS: CNPC_PlayerCompanion
//
//-----------------------------------------------------------------------------
#ifdef MAPBASE
class CNPC_PlayerCompanion : public CAI_GrenadeUser<CAI_PlayerAlly>
{
	DECLARE_CLASS( CNPC_PlayerCompanion, CAI_GrenadeUser<CAI_PlayerAlly> );
#else
class CNPC_PlayerCompanion : public CAI_PlayerAlly
{
	DECLARE_CLASS( CNPC_PlayerCompanion, CAI_PlayerAlly );
#endif
public:

	CNPC_PlayerCompanion();

	//---------------------------------
	bool			CreateBehaviors();
	void			Precache();
	void			Spawn();
	virtual void	SelectModel() {};

	virtual int		Restore( IRestore &restore );
	virtual void	DoCustomSpeechAI( void );

	//---------------------------------
	int 			ObjectCaps();
	bool 			ShouldAlwaysThink();

	Disposition_t	IRelationType( CBaseEntity *pTarget );
	
	bool			IsSilentSquadMember() const;

	//---------------------------------
	// Behavior
	//---------------------------------
	void 			GatherConditions();
	virtual void	PredictPlayerPush();
	void			BuildScheduleTestBits();

	CSound			*GetBestSound( int validTypes = ALL_SOUNDS );
	bool			QueryHearSound( CSound *pSound );
	bool			QuerySeeEntity( CBaseEntity *pEntity, bool bOnlyHateOrFearIfNPC = false );
	bool			ShouldIgnoreSound( CSound * );
	
	int 			SelectSchedule();

	virtual int 	SelectScheduleDanger();
	virtual int 	SelectSchedulePriorityAction();
	virtual int 	SelectScheduleNonCombat()			{ return SCHED_NONE; }
	virtual int 	SelectScheduleCombat();
	int 			SelectSchedulePlayerPush();

#ifdef EZ
	virtual int		SelectRangeAttack2Schedule();
#endif

	virtual bool	CanReload( void );

	virtual bool	ShouldDeferToFollowBehavior();
	bool			ShouldDeferToPassengerBehavior( void );

	bool			IsValidReasonableFacing( const Vector &vecSightDir, float sightDist );
	
	int 			TranslateSchedule( int scheduleType );
	
	void 			StartTask( const Task_t *pTask );
	void 			RunTask( const Task_t *pTask );
	
	Activity		TranslateActivityReadiness( Activity activity );
	Activity		NPC_TranslateActivity( Activity eNewActivity );
	void			OnChangeActivity( Activity eNewActivity );
	void 			HandleAnimEvent( animevent_t *pEvent );
	bool			HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);

	int				GetSoundInterests();
	
	void 			Touch( CBaseEntity *pOther );

	virtual bool	IgnorePlayerPushing( void );

	void			ModifyOrAppendCriteria( AI_CriteriaSet& set );
	void			Activate( void );

	void			PrepareReadinessRemap( void );
	
	virtual bool	IsNavigationUrgent( void );

	//---------------------------------
	// Readiness
	//---------------------------------

protected:
	virtual bool	IsReadinessCapable();
	bool			IsReadinessLocked() { return gpGlobals->curtime < m_flReadinessLockedUntil; }
	void			AddReadiness( float flAdd, bool bOverrideLock = false );
	void			SubtractReadiness( float flAdd, bool bOverrideLock = false );
	void			SetReadinessValue( float flSet );
	void			SetReadinessSensitivity( float flSensitivity ) { m_flReadinessSensitivity = flSensitivity; }
	virtual void	UpdateReadiness();
	virtual float	GetReadinessDecay();
	bool			IsInScriptedReadinessState( void ) { return (m_flReadiness < 0 ); }

	CUtlVector< CCompanionActivityRemap > m_activityMappings;

public:
	float			GetReadinessValue()	{ return m_flReadiness; }
	int				GetReadinessLevel();
	void			SetReadinessLevel( int iLevel, bool bOverrideLock, bool bSlam );
	void			LockReadiness( float duration = -1.0f ); // Defaults to indefinitely locked
	void			UnlockReadiness( void );

	virtual			void ReadinessLevelChanged( int iPriorLevel ) { 	}

#ifndef MAPBASE
	void			InputGiveWeapon( inputdata_t &inputdata );
#endif

#ifdef HL2_EPISODIC
	//---------------------------------
	// Vehicle passenger
	//---------------------------------
	void			InputEnterVehicle( inputdata_t &inputdata );
	void			InputEnterVehicleImmediately( inputdata_t &inputdata );
	void			InputCancelEnterVehicle( inputdata_t &inputdata );
	void			InputExitVehicle( inputdata_t &inputdata );
	bool			CanEnterVehicle( void );
	bool			CanExitVehicle( void );
	void			EnterVehicle( CBaseEntity *pEntityVehicle, bool bImmediately );
	virtual bool	ExitVehicle( void );

	virtual void	UpdateEfficiency( bool bInPVS );
	virtual bool	IsInAVehicle( void ) const;
	virtual	IServerVehicle *GetVehicle( void );
	virtual CBaseEntity *GetVehicleEntity( void );

	virtual bool CanRunAScriptedNPCInteraction( bool bForced = false );
	virtual bool IsAllowedToDodge( void );

#endif // HL2_EPISODIC

public:

	virtual void	OnPlayerKilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info );
#ifdef MAPBASE
	// This is just here to overwrite ai_playerally's TLK_ENEMY_DEAD
	virtual void	OnKilledNPC(CBaseCombatCharacter *pKilled) {}

	virtual void	Event_Killed( const CTakeDamageInfo &info );
	virtual void	Event_KilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info );
	virtual void	EnemyIgnited( CAI_BaseNPC *pVictim );
	virtual void	DoCustomCombatAI( void );
#endif

	//---------------------------------
	//---------------------------------
	bool PickTacticalLookTarget( AILookTargetArgs_t *pArgs );

	//---------------------------------
	// Aiming
	//---------------------------------
	CBaseEntity		*GetAimTarget() { return m_hAimTarget; }
	void			SetAimTarget( CBaseEntity *pTarget );
	void			StopAiming( char *pszReason = NULL );
	bool			FindNewAimTarget();
	void			OnNewLookTarget();
	bool			ShouldBeAiming();
	virtual bool	IsAllowedToAim();
	bool			HasAimLOS( CBaseEntity *pAimTarget );
	void			AimGun();
	CBaseEntity		*GetAlternateMoveShootTarget();

	//---------------------------------
	// Combat
	//---------------------------------
	virtual void 	LocateEnemySound() {};

	bool			IsValidEnemy( CBaseEntity *pEnemy );

	bool 			IsSafeFromFloorTurret( const Vector &vecLocation, CBaseEntity *pTurret );

	bool			ShouldMoveAndShoot( void );
	void			OnUpdateShotRegulator();

	void			DecalTrace( trace_t *pTrace, char const *decalName );
	bool 			FCanCheckAttacks();
	Vector 			GetActualShootPosition( const Vector &shootOrigin );
	WeaponProficiency_t CalcWeaponProficiency( CBaseCombatWeapon *pWeapon );
	bool			ShouldLookForBetterWeapon();
	bool			Weapon_CanUse( CBaseCombatWeapon *pWeapon );
	void			Weapon_Equip( CBaseCombatWeapon *pWeapon );
#ifdef MAPBASE
	bool			DoUnholster( void );
#endif
	void			PickupWeapon( CBaseCombatWeapon *pWeapon );

#if COMPANION_MELEE_ATTACK
	bool			KeyValue( const char *szKeyName, const char *szValue );
	int				MeleeAttack1Conditions( float flDot, float flDist );
#endif
	
	bool 			FindCoverPos( CBaseEntity *pEntity, Vector *pResult);
	bool			FindCoverPosInRadius( CBaseEntity *pEntity, const Vector &goalPos, float coverRadius, Vector *pResult );
	bool			FindCoverPos( CSound *pSound, Vector *pResult );
	bool			FindMortarCoverPos( CSound *pSound, Vector *pResult );
	bool 			IsCoverPosition( const Vector &vecThreat, const Vector &vecPosition );

	bool			IsEnemyTurret() { return ( GetEnemy() && IsTurret(GetEnemy()) ); }
	
	static bool		IsMortar( CBaseEntity *pEntity );
	static bool		IsSniper( CBaseEntity *pEntity );
	static bool		IsTurret(  CBaseEntity *pEntity );
	static bool		IsGunship( CBaseEntity *pEntity );
#ifdef EZ
	virtual bool		UseAttackSquadSlots() { return !IsCommandable(); } // All non-commandable "companions" should use attack squad slots
#endif
	//---------------------------------
	// Damage handling
	//---------------------------------
	int 			OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void 			OnFriendDamaged( CBaseCombatCharacter *pSquadmate, CBaseEntity *pAttacker );

	//---------------------------------
	// Hints
	//---------------------------------
	bool			FValidateHintType ( CAI_Hint *pHint );

	//---------------------------------
	// Navigation
	//---------------------------------
	bool			IsValidMoveAwayDest( const Vector &vecDest );
	bool 			ValidateNavGoal();
	bool 			OverrideMove( float flInterval );				// Override to take total control of movement (return true if done so)
	bool			MovementCost( int moveType, const Vector &vecStart, const Vector &vecEnd, float *pCost );
#ifdef EZ2
	virtual float	HintCost( int iHint, float dist, Vector &vecEnd );
#endif
	float			GetIdealSpeed() const;
	float			GetIdealAccel() const;
	bool			OnObstructionPreSteer( AILocalMoveGoal_t *pMoveGoal, float distClear, AIMoveResult_t *pResult );
#ifdef EZ
	bool			IsJumpLegal(const Vector &startPos, const Vector &apex, const Vector &endPos, float maxUp, float maxDown, float maxDist) const; // For inheritance reasons, need to pass this through to base class
	bool			IsJumpLegal(const Vector & startPos, const Vector & apex, const Vector & endPos) const; // Added by 1upD - all 'player companions' should be able to jump
#endif

	//---------------------------------
	// Inputs
	//---------------------------------
	void 			InputOutsideTransition( inputdata_t &inputdata );
	void			InputSetReadinessPanic( inputdata_t &inputdata );
	void			InputSetReadinessStealth( inputdata_t &inputdata );
	void			InputSetReadinessLow( inputdata_t &inputdata );
	void			InputSetReadinessMedium( inputdata_t &inputdata );
	void			InputSetReadinessHigh( inputdata_t &inputdata );
	void			InputLockReadiness( inputdata_t &inputdata );
#if HL2_EPISODIC
	void			InputClearAllOuputs( inputdata_t &inputdata ); ///< annihilate every output on this npc
#endif

	bool			AllowReadinessValueChange( void );

#ifdef MAPBASE
	virtual bool IsAltFireCapable() { return (m_iGrenadeCapabilities & GRENCAP_ALTFIRE) != 0 && BaseClass::IsAltFireCapable(); }
	virtual bool IsGrenadeCapable() { return (m_iGrenadeCapabilities & GRENCAP_GRENADE) != 0; }

	virtual bool	ShouldDropGrenades() { return (m_iGrenadeDropCapabilities & GRENDROPCAP_GRENADE) != 0 && BaseClass::ShouldDropGrenades(); }
	virtual bool	ShouldDropInterruptedGrenades() { return (m_iGrenadeDropCapabilities & GRENDROPCAP_INTERRUPTED) != 0 && BaseClass::ShouldDropInterruptedGrenades(); }
	virtual bool	ShouldDropAltFire() { return (m_iGrenadeDropCapabilities & GRENDROPCAP_ALTFIRE) != 0 && BaseClass::ShouldDropAltFire(); }

private:

	// Determines whether this NPC is allowed to use grenades or alt-fire stuff.
	eGrenadeCapabilities m_iGrenadeCapabilities;
	eGrenadeDropCapabilities m_iGrenadeDropCapabilities;
#endif

protected:
	//-----------------------------------------------------
	// Conditions, Schedules, Tasks
	//-----------------------------------------------------
	enum
	{
		COND_PC_HURTBYFIRE = BaseClass::NEXT_CONDITION,
		COND_PC_SAFE_FROM_MORTAR,
		COND_PC_BECOMING_PASSENGER,
		NEXT_CONDITION,

		SCHED_PC_COWER = BaseClass::NEXT_SCHEDULE,
		SCHED_PC_MOVE_TOWARDS_COVER_FROM_BEST_SOUND,
		SCHED_PC_TAKE_COVER_FROM_BEST_SOUND,
		SCHED_PC_FLEE_FROM_BEST_SOUND,
		SCHED_PC_FAIL_TAKE_COVER_TURRET,
		SCHED_PC_FAKEOUT_MORTAR,
		SCHED_PC_GET_OFF_COMPANION,
#ifdef COMPANION_MELEE_ATTACK
		SCHED_PC_MELEE_AND_MOVE_AWAY,
#endif
#ifdef MAPBASE
		SCHED_PC_AR2_ALTFIRE,
		SCHED_PC_MOVE_TO_FORCED_GREN_LOS,
		SCHED_PC_FORCED_GRENADE_THROW,
		SCHED_PC_RANGE_ATTACK2,		// Grenade throw
#endif
		NEXT_SCHEDULE,

		TASK_PC_WAITOUT_MORTAR = BaseClass::NEXT_TASK,
		TASK_PC_GET_PATH_OFF_COMPANION,
#ifdef MAPBASE
		TASK_PC_PLAY_SEQUENCE_FACE_ALTFIRE_TARGET,
		TASK_PC_GET_PATH_TO_FORCED_GREN_LOS,
		TASK_PC_DEFER_SQUAD_GRENADES,
		TASK_PC_FACE_TOSS_DIR,
#endif
		NEXT_TASK,

		AE_PC_MELEE = LAST_SHARED_ANIMEVENT

	};

private:
	void SetupCoverSearch( CBaseEntity *pEntity );
	void CleanupCoverSearch();

	//-----------------------------------------------------
	
	bool			m_bMovingAwayFromPlayer;
	bool			m_bWeightPathsInCover;

	enum eCoverType
	{
		CT_NORMAL,
		CT_TURRET,
		CT_MORTAR
	};

	static eCoverType	gm_fCoverSearchType;
	static bool 		gm_bFindingCoverFromAllEnemies;

	CSimpleSimTimer		m_FakeOutMortarTimer;

#ifdef EZ
// In EZ, soldiers need to access GetExpresser()
protected:
#endif
	// Derived classes should not use the expresser directly
	virtual CAI_Expresser *GetExpresser()	{ return BaseClass::GetExpresser(); }

protected:
	//-----------------------------------------------------

	virtual CAI_FollowBehavior &GetFollowBehavior( void ) { return m_FollowBehavior; }
	virtual CAI_StandoffBehavior &GetStandoffBehavior( void ) { return m_StandoffBehavior; } // Blixibon - Added because soldiers have their own special standoff behavior
#ifdef EZ2
	virtual CAI_SurrenderBehavior &GetSurrenderBehavior( void ) { return m_SurrenderBehavior; }
#endif

	CAI_AssaultBehavior				m_AssaultBehavior;
	CAI_FollowBehavior				m_FollowBehavior;
	CAI_StandoffBehavior			m_StandoffBehavior;
	CAI_LeadBehavior				m_LeadBehavior;
	CAI_ActBusyBehavior				m_ActBusyBehavior;
#ifdef HL2_EPISODIC
	CAI_OperatorBehavior			m_OperatorBehavior;
	CAI_PassengerBehaviorCompanion	m_PassengerBehavior;
	CAI_FearBehavior				m_FearBehavior;
#endif
#ifdef MAPBASE
	CAI_FuncTankBehavior			m_FuncTankBehavior;
#endif
#ifdef EZ2
	CAI_SurrenderBehavior			m_SurrenderBehavior;
#endif
	//-----------------------------------------------------

	bool	ShouldAlwaysTransition( void );

	// Readiness is a value that's fed by various events in the NPC's AI. It is used
	// to make decisions about what type of posture the NPC should be in (relaxed, agitated).
	// It is not used to make decisions about what to do in the AI. 
	float m_flReadiness;
	float m_flReadinessSensitivity;
	bool m_bReadinessCapable;
	float m_flReadinessLockedUntil;
	float	m_fLastBarrelExploded;
	float	m_fLastPlayerKill;
	int		m_iNumConsecutiveBarrelsExploded;  // Companions keep track of the # of consecutive barrels exploded by the player and speaks a response as it increases
	int		m_iNumConsecutivePlayerKills;  // Alyx keeps track of the # of consecutive kills by the player and speaks a response as it increases

	//-----------------------------------------------------

	float m_flBoostSpeed;

	//-----------------------------------------------------
	
	CSimpleSimTimer m_AnnounceAttackTimer;

	//-----------------------------------------------------

	EHANDLE m_hAimTarget;

#ifdef HL2_EPISODIC
	CHandle<CPhysicsProp>	m_hFlare;
#endif // HL2_EPISODIC

	//-----------------------------------------------------

#ifdef MAPBASE
	static string_t gm_iszMortarClassname;
	#define gm_iszFloorTurretClassname gm_isz_class_FloorTurret
	static string_t gm_iszGroundTurretClassname;
	#define gm_iszShotgunClassname gm_isz_class_Shotgun
	#define gm_iszRollerMineClassname gm_isz_class_Rollermine
	#define gm_iszSMG1Classname gm_isz_class_SMG1
	#define gm_iszAR2Classname gm_isz_class_AR2
#else
	static string_t gm_iszMortarClassname;
	static string_t gm_iszFloorTurretClassname;
	static string_t gm_iszGroundTurretClassname;
	static string_t gm_iszShotgunClassname;
	static string_t	gm_iszRollerMineClassname;
#ifdef MAPBASE
	static string_t gm_iszSMG1Classname;
	static string_t gm_iszAR2Classname;
#endif
#endif

	//-----------------------------------------------------

	void	InputEnableAlwaysTransition( inputdata_t &inputdata );
	void	InputDisableAlwaysTransition( inputdata_t &inputdata );
	bool	m_bAlwaysTransition;
	bool	m_bDontPickupWeapons;

	void	InputEnableWeaponPickup( inputdata_t &inputdata );
	void	InputDisableWeaponPickup( inputdata_t &inputdata );

	COutputEvent	m_OnWeaponPickup;

#if COMPANION_MELEE_ATTACK
	int		m_nMeleeDamage;
#endif

	CStopwatch		m_SpeechWatch_PlayerLooking;

	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;
};

#ifdef EZ
//-----------------------------------------------------------------------------
// Blixibon - Moved to CNPC_PlayerCompanion so soldiers can use it
//-----------------------------------------------------------------------------
struct SquadMemberInfo_t
{
	CNPC_PlayerCompanion *pMember;
	bool			bSeesPlayer;
	float			distSq;
};

int __cdecl SquadSortFunc( const SquadMemberInfo_t *pLeft, const SquadMemberInfo_t *pRight );
#endif

// Used for quick override move searches against certain types of entities
void OverrideMoveCache_ForceRepopulateList( void );
CBaseEntity *OverrideMoveCache_FindTargetsInRadius( CBaseEntity *pFirstEntity, const Vector &vecOrigin, float flRadius );
void OverrideMoveCache_LevelInitPreEntity( void );
void OverrideMoveCache_LevelShutdownPostEntity( void );

#endif // NPC_PLAYERCOMPANION_H
