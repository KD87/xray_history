#pragma once

#include "ai_monster_shared_data.h"
#include "../../../SkeletonAnimated.h"

class		CBaseMonster;
class		CJumping;
class		CAnimTriple;

//////////////////////////////////////////////////////////////////////////
class CMotionManager : public CSharedClass<_motion_shared, CLASS_ID> {

	REPLACED_ANIM			m_tReplacedAnims;	// �������� �������

	CBaseMonster			*pMonster;
	CJumping				*pJumping;

	// ������ � ��������������������
	SEQ_VECTOR				seq_states;
	SEQ_VECTOR_IT			seq_it;				
	bool					seq_playing;

	// ����������� �������� 
	EMotionAnim				prev_motion; 

	// ����������� �������� '���� �� �����'
	TTime					time_start_stand;
	EAction					prev_action;

	// ������ � ���������� �����
	TTime					aa_time_last_attack;	// ����� ���������� ��������� ����

	// -------------------------------------------------------------------------

	u32						spec_params;			// �������������� ���������

	bool					transition_sequence_used;	// ��������� �������� ����� ��������
	EMotionAnim				target_transition_anim;

	TTime					fx_time_last_play;

	bool					bad_motion_fixed;		// true, ���� ������ �������� ���������, �� ����� �� �����

	// -------------------------------------------------------------------------------------
	// Acceleration
	
	struct {
		bool					active;				
		bool					enable_braking;	// �� ������������ ��� ����������

		EAccelType				type;	
		
		float					calm;
		float					aggressive;

		VELOCITY_CHAIN_VEC		chain;
	} m_accel;

	// ---------------------------------------------------------------------------------------

	CAnimTriple					*pCurAnimTriple;

private:

	ANIM_TO_MOTION_MAP			m_anim_motion_map;

public:

	EAction					m_tAction;
	MotionID				m_tpCurAnim;

	float					m_prev_character_velocity;

public:

				CMotionManager			();
				~CMotionManager			();

	void		reinit					();
	void		Init					(CBaseMonster	*pM);

	// �������� ����� �������� (��������� �� Monster::Load)
	void		AddAnim					(EMotionAnim ma, LPCSTR tn, int s_id, SVelocityParam *vel, EPState p_s);
	void		AddAnim					(EMotionAnim ma, LPCSTR tn, int s_id, SVelocityParam *vel, EPState p_s, LPCSTR fx_front, LPCSTR fx_back, LPCSTR fx_left, LPCSTR fx_right);

	// -------------------------------------

	// �������� �������� �������� (A - Animation, S - Position)
	void		AddTransition			(EMotionAnim from,	EMotionAnim to, EMotionAnim trans, bool chain, bool skip_aggressive = false);
	void		AddTransition			(EMotionAnim from,	EPState to,		EMotionAnim trans, bool chain, bool skip_aggressive = false);
	void		AddTransition			(EPState from,		EMotionAnim to, EMotionAnim trans, bool chain, bool skip_aggressive = false);
	void		AddTransition			(EPState from,		EPState to,		EMotionAnim trans, bool chain, bool skip_aggressive = false);

	// -------------------------------------

	void		LinkAction				(EAction act, EMotionAnim pmt_motion, EMotionAnim pmt_left, EMotionAnim pmt_right, float pmt_angle);
	void		LinkAction				(EAction act, EMotionAnim pmt_motion);

	// -------------------------------------

	void		AddReplacedAnim			(bool *b_flag, EMotionAnim pmt_cur_anim, EMotionAnim pmt_new_anim);
	
	// -------------------------------------

	void		FrameUpdate				();
	void		UpdateScheduled			();
	void		ScheduledInit			();

	// ����������� ������� �������� �� ������ �� SelectAnimation
	bool		PrepareAnimation		();

	void		CheckTransition			(EMotionAnim from, EMotionAnim to);

	void		OnAnimationEnd			();
	void		ShowDeath				();

	void		SetSpecParams			(u32 param) {spec_params |= param;}
	void		SetCurAnim				(EMotionAnim a) {cur_anim_info().motion = a;}
	EMotionAnim	GetCurAnim				() {return  cur_anim_info().motion;} 

	// ������ � ��������������������
	void		Seq_Init				();
	void		Seq_Add					(EMotionAnim a);
	void		Seq_Switch				();					// ������� � ��������� ���������, ���� �������� �� ������� - ���������
	void		Seq_Finish				();
	EMotionAnim	Seq_CurAnim				() {return ((seq_playing) ? *seq_it : eAnimStandIdle );}
	bool		Seq_Active				() {return seq_playing;}

	// ������ � ���������� ����
	void		AA_Load					(LPCSTR section);
	bool		AA_TimeTest				(SAAParam &params);
	void		AA_UpdateLastAttack		(TTime cur_time) {aa_time_last_attack = cur_time;}
	void		AA_GetParams			(SAAParam &params, LPCSTR anim_name);

	// FX's
	void		FX_Play					(EHitSide side, float amount);
	
	bool		BadMotionFixed			() {return bad_motion_fixed;}

private:	

	void		UpdateAnimCount			();
	
	// ������ � ���������� ����
	void		AA_Clear				(); 
	void		AA_SwitchAnimation		(EMotionAnim a, u32 i3);

	// �������������� �������
	EPState		GetState				(EMotionAnim a);

	void		CheckReplacedAnim		();

	CMotionDef	*get_motion_def			(ANIM_ITEM_MAP_IT &it, u32 index);

public:

	// ����� PrepareAnimation() � ��������� ��������
	void		ForceAnimSelect			();
	
	float		GetCurAnimTime			();
	float		GetAnimSpeed			(EMotionAnim anim);

	bool		IsStandCurAnim			();

	void		ValidateAnimation		();

	//////////////////////////////////////////////////////////////////////////

	void		Update					();

private:
	void		SelectAnimation			();
	void		SelectVelocities		();
	
	EAction		GetActionFromPath		();
	EAction		VelocityIndex2Action	(u32 velocity_index);
	
//////////////////////////////////////////////////////////////////////////
// DEBUG

private:
	LPCSTR		GetAnimationName		(EMotionAnim anim);
	LPCSTR		GetActionName			(EAction action);

// end DEBUG
//////////////////////////////////////////////////////////////////////////

public:
	//-------------------------------------------------------------------------------
	// Acceleration
		
		void	accel_init				();
		void	accel_load				(LPCSTR section);

		void	accel_activate			(EAccelType type);
	IC	void	accel_deactivate		() {m_accel.active = false;	m_accel.enable_braking = false;}
	IC	void	accel_set_braking		(bool val = true) {m_accel.enable_braking = val;}

		float	accel_get				(EAccelValue val = eAV_Accel);

	IC	bool	accel_active			(EAccelValue val = eAV_Accel) {return (val == eAV_Accel) ? m_accel.active : m_accel.enable_braking;}

		void	accel_chain_add			(EMotionAnim anim1, EMotionAnim anim2);
		bool	accel_chain_get			(float cur_speed, EMotionAnim target_anim, EMotionAnim &new_anim, float &a_speed);
		bool	accel_chain_test		();

		bool	accel_check_braking		(float before_interval);

	//-------------------------------------------------------------------------------
	// Triple Animation
	
		void	TA_Activate				(CAnimTriple *p_triple, bool skip_prepare = false);
		void	TA_Deactivate			();
		void	TA_PointBreak			();
		bool	TA_IsActive				();


	// -----------------------------------------------------
	// Jumps
		
		void	ActivateJump			();
		void	DeactivateJump			();
		bool	JumpActive				();

	// Critical Action

		bool	IsCriticalAction		();

		void	CheckVelocityBounce		();

	// Other
		void	SetTurnAnimation		();

	// MotionDef to animation name translation
		void		AddAnimTranslation		(const MotionID &motion, LPCSTR str);
		shared_str	GetAnimTranslation		(const MotionID &motion);
public:
		
		// ���������� � ������� ��������
		SCurrentAnimationInfo	m_cur_anim;
		SCurrentAnimationInfo	&cur_anim_info() {return m_cur_anim;}

};

