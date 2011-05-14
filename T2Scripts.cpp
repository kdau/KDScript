/******************************************************************************
 *  T2Scripts.cpp
 *
 *  This file is part of Public Scripts
 *  Copyright (C) 2005-2011 Tom N Harris <telliamed@whoopdedo.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/
#if (_DARKGAME == 2)
#include "T2Scripts.h"
#include "ScriptModule.h"

#include <lg/types.h>
#include <lg/interface.h>
#include <lg/scrservices.h>
#include <lg/propdefs.h>
#include <lg/objects.h>
#include <ScriptLib.h>
#include "utils.h"

#include <cstring>
#include <cstdlib>
#include <cctype>
#include <cmath>

using namespace std;


/***
 * Burplauncher
 */
int cScr_SlayFirer::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript* pScript, void*)
{
	SService<IDamageSrv> pDS(g_pScriptManager);
	sLink sl;
	pLQ->Link(&sl);
	pDS->Slay(sl.dest, static_cast<cScr_SlayFirer*>(pScript)->ObjId());
	return 1;
}

long cScr_SlayFirer::OnSlain(sSlayMsg* pSlayMsg, cMultiParm& mpReply)
{
	IterateLinks("~Firer", ObjId(), 0, LinkIter, this, NULL);

	return cBaseScript::OnSlain(pSlayMsg, mpReply);
}


/***
 * CollapseFloor
 */
long cScr_CollapseFloor::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "Collapse"))
	{
		SInterface<IDamageModel> pDmg(g_pScriptManager);
		pDmg->SlayObject(ObjId(),ObjId(), NULL);
		return 0;
	}
	return cBasePPlateScript::OnMessage(pMsg, mpReply);
}

long cScr_CollapseFloor::OnPressurePlateActive(sScrMsg* pMsg, cMultiParm& mpReply)
{
	// Pressure plate physics interferes with flinders.
	// So wait for the object to come to a complete stop before killing it.
	PostMessage(ObjId(), "Collapse");
	return cBasePPlateScript::OnPressurePlateActive(pMsg, mpReply);
}


/***
 * Elemental
 */
void cScr_Elemental::UpdateScale(void)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "MAX_HP") && pPS->Possessed(ObjId(), "HitPoints"))
	{
		cMultiParm mpProp;
		int iHP, iMaxHP;
		pPS->Get(mpProp, ObjId(), "MAX_HP", NULL);
		iMaxHP = static_cast<int>(mpProp);
		pPS->Get(mpProp, ObjId(), "HitPoints", NULL);
		iHP = static_cast<int>(mpProp);
		double fScale = float(iHP)/iMaxHP;
		if (fScale < 0.375)
			fScale = 0.375;
		fScale = CalculateCurve(fScale, 0, 1, object(ObjId()));

		mxs_vector vScale;
		vScale.z = vScale.y = vScale.x = fScale;
		mpProp.Unset();
		mpProp.pVector = &vScale;
		mpProp.type = kMT_Vector;
		if (!pPS->Possessed(ObjId(), "Scale"))
			pPS->Add(ObjId(), "Scale");
		pPS->SetSimple(ObjId(), "Scale", mpProp);
		mpProp.type = kMT_Undef;
	}
}

void cScr_Elemental::NotifyParticles(const char* pszMsg)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	pLS->BroadcastOnAllLinks(ObjId(), pszMsg, pLTS->LinkKindNamed("~ParticleAttachement"));
}

long cScr_Elemental::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
	if (pSimMsg->fStarting)
		UpdateScale();

	return cBaseScript::OnSim(pSimMsg, mpReply);
}

long cScr_Elemental::OnSlain(sSlayMsg* pSlayMsg, cMultiParm& mpReply)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "StTweqScale"))
	{
		cMultiParm mpProp;
		cScrVec vScale(1.0,1.0,1.0);
		cScrVec vCfg;
		if (pPS->Possessed(ObjId(), "Scale"))
		{
			pPS->Get(mpProp, ObjId(), "Scale", NULL);
			vScale = *static_cast<const mxs_vector*>(mpProp);
		}
		pPS->Add(ObjId(), "CfgTweqScale");
		vCfg.x = vScale.x;
		vCfg.y = 0;
		vCfg.z = vScale.x;
		mpProp = vCfg;
		pPS->Set(ObjId(), "CfgTweqScale", "x rate-low-high", mpProp);
		vCfg.z = vScale.y;
		mpProp = vCfg;
		pPS->Set(ObjId(), "CfgTweqScale", "y rate-low-high", mpProp);
		vCfg.z = vScale.z;
		mpProp = vCfg;
		pPS->Set(ObjId(), "CfgTweqScale", "z rate-low-high", mpProp);

		mpProp = kTweqFlagOn|kTweqFlagReverse;
		pPS->Set(ObjId(), "StTweqScale", "Axis 1AnimS", mpProp);
		pPS->Set(ObjId(), "StTweqScale", "Axis 2AnimS", mpProp);
		pPS->Set(ObjId(), "StTweqScale", "Axis 3AnimS", mpProp);
		pPS->Set(ObjId(), "StTweqScale", "AnimS", mpProp);

		NotifyParticles("Die");
	}

	return cBaseScript::OnSlain(pSlayMsg, mpReply);
}

long cScr_Elemental::OnDamage(sDamageScrMsg* pDmgMsg, cMultiParm& mpReply)
{
	UpdateScale();

	return cBaseScript::OnDamage(pDmgMsg, mpReply);
}

long cScr_Elemental::OnTweqComplete(sTweqMsg* pTweqMsg, cMultiParm& mpReply)
{
	if (pTweqMsg->Type == kTweqTypeScale)
	{
		NotifyParticles("Die");
	}

	return cBaseScript::OnTweqComplete(pTweqMsg, mpReply);
}


/***
 * FireElement
 */
long cScr_FireElement::OnAlertness(sAIAlertnessMsg* pAlertMsg, cMultiParm& mpReply)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	const char* pszMoodMsg = "Mood0";
	float fExtraLight = 0.6f;
	float fAlpha = 0.3f;
	int iDynLight = 25;

	switch (pAlertMsg->level)
	{
	case kNoAlert:
		break;
	case kLowAlert:
		fExtraLight = 11.0f/15.0f;
		fAlpha = 0.5f;
		iDynLight = 50;
		pszMoodMsg = "Mood1";
		break;
	case kModerateAlert:
		fExtraLight = 13.0f/15.0f;
		fAlpha = 0.7f;
		iDynLight = 75;
		pszMoodMsg = "Mood2";
		break;
	case kHighAlert:
		fExtraLight = 1.0f;
		fAlpha = 0.9f;
		iDynLight = 100;
		pszMoodMsg = "Mood3";
		break;
	}

	cMultiParm mpProp;
	mpProp = fExtraLight;
	pPS->Set(ObjId(), "ExtraLight", "Amount (-1..1)", mpProp);
	mpProp = fAlpha;
	pPS->SetSimple(ObjId(), "RenderAlpha", mpProp);
	mpProp = iDynLight;
	pPS->SetSimple(ObjId(), "SelfLit", mpProp);

	if (pAlertMsg->level == kHighAlert && pAlertMsg->oldLevel < kHighAlert)
	{
		AddSingleMetaProperty("BigHeatSource", ObjId());
	}
	else if (pAlertMsg->level < kHighAlert && pAlertMsg->oldLevel == kHighAlert)
	{
		RemoveSingleMetaProperty("BigHeatSource", ObjId());
	}

	NotifyParticles(pszMoodMsg);

	return cScr_Elemental::OnAlertness(pAlertMsg, mpReply);
}

long cScr_FireElement::OnDamage(sDamageScrMsg* pDmgMsg, cMultiParm& mpReply)
{
	if (pDmgMsg->damage > 0)
	{
		SService<IObjectSrv> pOS(g_pScriptManager);
		object oObj, oHit;
		pOS->Named(oObj, "WaterStim");
		if (oObj && pDmgMsg->kind == oObj)
		{
			pOS->Named(oHit, "SteamPuff");
		}
		else
		{
			pOS->Named(oHit, "FElemHit");
		}
		if (oHit)
		{
			pOS->Create(oObj, oHit);
			if (oObj)
				pOS->Teleport(oObj, cScrVec::Zero, cScrVec::Zero, ObjId());
		}
	}

	return cScr_Elemental::OnDamage(pDmgMsg, mpReply);
}


/***
 * FireElemSparx
 */
void cScr_FireSparx::SetColor(int iColor)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "ParticleGroup"))
	{
		pPS->Set(ObjId(), "ParticleGroup", "color (palettized)", iColor);
	}
}

long cScr_FireSparx::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "Die"))
	{
		SService<IObjectSrv> pOS(g_pScriptManager);
		pOS->Destroy(ObjId());
		return 0;
	}
	else if (!_stricmp(pMsg->message, "Mood0"))
	{
		SetColor(185);
		return 0;
	}
	else if (!_stricmp(pMsg->message, "Mood1") || !_stricmp(pMsg->message, "Mood2"))
	{
		SetColor(187);
		return 0;
	}
	else if (!_stricmp(pMsg->message, "Mood3"))
	{
		SetColor(122);
		return 0;
	}
	return cBaseScript::OnMessage(pMsg, mpReply);
}

long cScr_FireSparx::OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPGroupSrv> pSFX(g_pScriptManager);
	pSFX->SetActive(ObjId(), TRUE);

	return cBaseScript::OnTurnOn(pMsg, mpReply);
}

long cScr_FireSparx::OnTurnOff(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPGroupSrv> pSFX(g_pScriptManager);
	pSFX->SetActive(ObjId(), FALSE);

	return cBaseScript::OnTurnOff(pMsg, mpReply);
}


/***
 * HotPlate
 */
cScr_HotPlate::cScr_HotPlate(const char* pszName, int iHostObjId)
	: cBaseScript(pszName, iHostObjId)
{
	static const sMessageHandler handlers[] = 
		{{"HotPlateHeat",HandleHotPlateHeat}, {"WaterStimStimulus",HandleWaterStimStimulus}};
	RegisterMessageHandlers(handlers, 2);
}

long cScr_HotPlate::HandleHotPlateHeat(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cScr_HotPlate*>(pScript)->OnHotPlateHeat(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cScr_HotPlate::HandleWaterStimStimulus(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cScr_HotPlate*>(pScript)->OnWaterStimStimulus(static_cast<sStimMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cScr_HotPlate::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->SubscribeMsg(ObjId(), kPhysContact);

	return cBaseScript::OnBeginScript(pMsg, mpReply);
}

long cScr_HotPlate::OnEndScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->UnsubscribeMsg(ObjId(), kPhysContact);

	return cBaseScript::OnEndScript(pMsg, mpReply);
}

long cScr_HotPlate::OnPhysContactCreate(sPhysMsg* pPhysMsg, cMultiParm& mpReply)
{
	SService<IActReactSrv> pARSrv(g_pScriptManager);
	pARSrv->EndContact(ObjId(), pPhysMsg->contactObj);
	pARSrv->BeginContact(ObjId(), pPhysMsg->contactObj);

	return cBaseScript::OnPhysContactCreate(pPhysMsg, mpReply);
}

long cScr_HotPlate::OnPhysContactDestroy(sPhysMsg* pPhysMsg, cMultiParm& mpReply)
{
	SService<IActReactSrv> pARSrv(g_pScriptManager);
	pARSrv->EndContact(ObjId(), pPhysMsg->contactObj);

	return cBaseScript::OnPhysContactDestroy(pPhysMsg, mpReply);
}

long cScr_HotPlate::OnHotPlateHeat(sScrMsg* pMsg, cMultiParm&)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	float fHeat = static_cast<float>(pMsg->data);
	if (IsScriptDataSet("m_bCool"))
	{
		cMultiParm mpCurHeat;
		if (!pPS->Possessed(ObjId(), "ExtraLight"))
			return 0;
		pPS->Get(mpCurHeat, ObjId(), "ExtraLight", "Amount (-1..1)");
		if (static_cast<float>(mpCurHeat) < fHeat)
			return 0;
	}
	if (!pPS->Possessed(ObjId(), "ExtraLight"))
		pPS->Add(ObjId(), "ExtraLight");
	pPS->Set(ObjId(), "ExtraLight", "Amount (-1..1)", fHeat);

	if (fHeat > 0)
		AddSingleMetaProperty("HotPlateHeat", ObjId());
	else
		RemoveSingleMetaProperty("HotPlateHeat", ObjId());

	return 0;
}

long cScr_HotPlate::OnWaterStimStimulus(sStimMsg*, cMultiParm&)
{
	SService<IObjectSrv> pOS(g_pScriptManager);
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (!IsScriptDataSet("m_bCool"))
	{
		SService<ILinkSrv> pLS(g_pScriptManager);
		SService<ILinkToolsSrv> pLTS(g_pScriptManager);
		link lControl, lNew;
		sLink sl;
		pLS->GetOne(lControl, pLTS->LinkKindNamed("~ControlDevice"), ObjId(), 0);
		if (!lControl)
		{
			pPS->Set(ObjId(), "ExtraLight", "Amount (-1..1)", -1);
			RemoveSingleMetaProperty("HotPlateHeat", ObjId());
			return 0;
		}
		pLTS->LinkGet(lControl, sl);
		object oClone;
		pOS->BeginCreate(oClone, sl.dest);
		if (!oClone)
		{
			pPS->Set(ObjId(), "ExtraLight", "Amount (-1..1)", -1);
			RemoveSingleMetaProperty("HotPlateHeat", ObjId());
			return 0;
		}
		cMultiParm mpJoint;
		pPS->Get(mpJoint, oClone, "CfgTweqJoints", "    rate-low-high");
		const_cast<mxs_vector*>(static_cast<const mxs_vector*>(mpJoint))->x =
			(static_cast<const mxs_vector*>(mpJoint)->z - static_cast<const mxs_vector*>(mpJoint)->y) * 0.5f;
		pPS->Set(oClone, "CfgTweqJoints", "    rate-low-high", mpJoint);
		pPS->Set(oClone, "CfgTweqJoints", "Halt", 0); // Destroy Obj
		pPS->Set(oClone, "StTweqJoints", "AnimS", 3); // Reverse
		pPS->Set(oClone, "StTweqJoints", "Joint1AnimS", 3);
		pPS->Set(oClone, "StTweqBlink", "AnimS", 1); // just in case
		pOS->EndCreate(oClone);
		pLS->Create(lNew, pLTS->LinkKindNamed("ControlDevice"), oClone, ObjId());
	}
	if (pPS->Possessed(ObjId(), "ScriptTiming"))
	{
		cMultiParm mpTiming;
		pPS->Get(mpTiming, ObjId(), "ScriptTiming", NULL);
		SetScriptData("m_bCool", mpTiming);
	}
	else
		SetScriptData("m_bCool", 1);
	return 0;
}

long cScr_HotPlate::OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (IsScriptDataSet("m_bCool"))
	{
		int iCool = static_cast<int>(GetScriptData("m_bCool"));
		if (iCool <= 0)
		{
			ClearScriptData("m_bCool");
		}
	}

	return cBaseScript::OnTurnOn(pMsg, mpReply);
}

long cScr_HotPlate::OnTurnOff(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (IsScriptDataSet("m_bCool"))
	{
		int iCool = GetScriptData("m_bCool");
		SetScriptData("m_bCool", iCool-1);
	}

	return cBaseScript::OnTurnOff(pMsg, mpReply);
}


/***
 * HotPlateControl
 */
long cScr_HotPlateCtrl::OnTweqComplete(sTweqMsg* pTweqMsg, cMultiParm& mpReply)
{
	if (pTweqMsg->Type == kTweqTypeFlicker
	 && pTweqMsg->Op == kTweqOpFrameEvent)
	{
		SService<IPropertySrv> pPS(g_pScriptManager);
		cMultiParm mpJoint, mpAnim;
		pPS->Get(mpJoint, ObjId(), "JointPos", "Joint 1");
		pPS->Get(mpAnim, ObjId(), "", "Joint1AnimS");

		CDSend("HotPlateHeat", ObjId(), mpJoint);

		int iReverse = bool(static_cast<int>(mpAnim) & kTweqFlagReverse);
		if (IsScriptDataSet("m_iReverse"))
		{
			int iFlags = GetScriptData("m_iReverse");
			if (iFlags != iReverse)
			{
				CDSend(iReverse?"TurnOff":"TurnOn", ObjId());
			}
		}
		SetScriptData("m_iReverse", iReverse);
	}

	return cBaseScript::OnTweqComplete(pTweqMsg, mpReply);
}

long cScr_HotPlateCtrl::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "Heat?"))
	{
		SService<IPropertySrv> pPS(g_pScriptManager);
		cMultiParm mpJoint;
		pPS->Get(mpJoint, ObjId(), "JointPos", "Joint 1");
		mpReply = mpJoint;
		return 0;
	}
	return cBaseScript::OnMessage(pMsg, mpReply);
}


/***
 * ModelByCount
 */
void cScr_ModelByCount::UpdateModel(void)
{
	if (!bUpdating)
	{
		bUpdating = true;
		SService<IPropertySrv> pPS(g_pScriptManager);
		if (pPS->Possessed(ObjId(), "CfgTweqModels"))
		{
			cMultiParm mpProp;
			int iCount = 0;
			if (pPS->Possessed(ObjId(), "StackCount"))
			{
				pPS->Get(mpProp, ObjId(), "StackCount", NULL);
				iCount = static_cast<int>(mpProp) - 1;
			}
			if (iCount < 0)
				iCount = 0;
			else if (iCount > 5)
				iCount = 5;

			char szModel[8];
			strcpy(szModel, "Model 0");
			mpProp = "";
			while (strlen(static_cast<const char*>(mpProp)) == 0 && iCount >= 0)
			{
				szModel[6] = '0' + iCount--;
				pPS->Get(mpProp, ObjId(), "CfgTweqModels", szModel);
			}
			if (iCount >= 0)
			{
				/*
				SService<ILinkSrv> pLS(g_pScriptManager);
				SService<ILinkToolsSrv> pLTS(g_pScriptManager);
				int iPlayer = StrToObject("Player");
				long lCont = pLTS->LinkKindNamed("Contains");
				true_bool bInv;
				pLS->AnyExist(bInv, lCont, iPlayer, ObjId());
				if (bInv)
				{
					link l;
					pLS->GetOne(l, lCont, iPlayer, ObjId());
					pLS->Destroy(l);
				}
				*/
				pPS->SetSimple(ObjId(), "ModelName", mpProp);
				/*
				if (bInv)
				{
					link l;
					pLS->Create(l, lCont, iPlayer, ObjId());
				}
				*/
			}
		}
		bUpdating = false;
	}
}

long cScr_ModelByCount::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
	if (pSimMsg->fStarting)
		UpdateModel();

	return cBaseScript::OnSim(pSimMsg, mpReply);
}

long cScr_ModelByCount::OnContained(sContainedScrMsg* pContMsg, cMultiParm& mpReply)
{
	if (pContMsg->event == kContainAdd
	 || pContMsg->event == kContainRemove)
		UpdateModel();

	return cBaseScript::OnContained(pContMsg, mpReply);
}

/*
long cScr_ModelByCount::OnCombine(sCombineScrMsg* pContMsg, cMultiParm& mpReply)
{
	//UpdateModel();

	return cBaseScript::OnCombine(pContMsg, mpReply);
}
*/

long cScr_ModelByCount::OnCreate(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (IsSim())
		UpdateModel();

	return cBaseScript::OnCreate(pMsg, mpReply);
}


/***
 * SecureDoor
 */
eDoorState cScr_SecureDoor::GetDoorState(void)
{
	static const eDoorState _State[] = 
		{ kDoorStateClosed, kDoorStateOpen, kDoorStateClosed, kDoorStateOpen, kDoorStateOpen, kDoorStateClosed };
	SService<IDoorSrv> pDoor(g_pScriptManager);
	return _State[pDoor->GetDoorState(ObjId())];
}

cAnsiStr cScr_SecureDoor::GetWatchers(void)
{
	cAnsiStr strWatchers;
	char* pszParam = GetObjectParamString(ObjId(), "watcher");
	if (pszParam)
	{
		strWatchers = "@";
		strWatchers += pszParam;
		g_pMalloc->Free(pszParam);
	}
	else
		strWatchers = "@Human";
	return strWatchers;
}

long cScr_SecureDoor::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
	if (pSimMsg->fStarting)
		SetTimedMessage("DelayInit", 220, kSTM_OneShot, "SimTrigger");

	return cBaseDoorScript::OnSim(pSimMsg, mpReply);
}

long cScr_SecureDoor::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (!strcmp(pTimerMsg->name, "DelayInit") && pTimerMsg->data == "SecureDoor")
	{
		if (!IsScriptDataSet("m_iInitState"))
			SetScriptData("m_iInitState", GetDoorState());
		return 0;
	}

	return cBaseDoorScript::OnTimer(pTimerMsg, mpReply);
}

long cScr_SecureDoor::OnDoorOpening(sDoorMsg* pDoorMsg, cMultiParm& mpReply)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	cAnsiStr strWatchers = GetWatchers();
	char szSelf[12];
	sprintf(szSelf, "%d", ObjId());
	int iInitState = GetScriptData("m_iInitState");
	pLS->DestroyMany(pLTS->LinkKindNamed("AIWatchObj"), static_cast<const char*>(strWatchers), szSelf);
	if (iInitState != kDoorStateOpen)
		pLS->CreateMany(pLTS->LinkKindNamed("AIWatchObj"), static_cast<const char*>(strWatchers), szSelf);

	return cBaseDoorScript::OnDoorOpening(pDoorMsg, mpReply);
}

long cScr_SecureDoor::OnDoorClose(sDoorMsg* pDoorMsg, cMultiParm& mpReply)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	cAnsiStr strWatchers = GetWatchers();
	char szSelf[12];
	sprintf(szSelf, "%d", ObjId());
	int iInitState = GetScriptData("m_iInitState");
	pLS->DestroyMany(pLTS->LinkKindNamed("AIWatchObj"), static_cast<const char*>(strWatchers), szSelf);
	if (iInitState != kDoorStateClosed)
		pLS->CreateMany(pLTS->LinkKindNamed("AIWatchObj"), static_cast<const char*>(strWatchers), szSelf);

	return cBaseDoorScript::OnDoorClose(pDoorMsg, mpReply);
}


/***
 * StickyVines
 */
long cScr_VineShot::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->SubscribeMsg(ObjId(), kPhysCollision);

	return cBaseScript::OnBeginScript(pMsg, mpReply);
}

long cScr_VineShot::OnEndScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->UnsubscribeMsg(ObjId(), kPhysCollision);

	return cBaseScript::OnEndScript(pMsg, mpReply);
}

long cScr_VineShot::OnPhysCollision(sPhysMsg* pPhysMsg, cMultiParm& mpReply)
{
	object iPlayer = StrToObject("Player");
	if (pPhysMsg->collObj == iPlayer)
	{
		SService<IObjectSrv> pOS(g_pScriptManager);
		object oVines, oJunk;
		pOS->Named(oVines, "JunkEarthWebs");
		if (oVines)
		{
			pOS->Create(oJunk, oVines);
			if (oJunk)
			{
				SService<IContainSrv> pCS(g_pScriptManager);
				pCS->Add(oJunk, iPlayer, 0, 1);
			}
		}
	}
	mpReply = sPhysMsg::kSlay;

	return cBaseScript::OnPhysCollision(pPhysMsg, mpReply);
}


/***
 * JunkVines
 */
long cScr_JunkVines::OnContained(sContainedScrMsg* pContMsg, cMultiParm& mpReply)
{
	int iPlayer = StrToObject("Player");
	if (pContMsg->container == iPlayer)
	{
		SService<IActReactSrv> pARSrv(g_pScriptManager);
		if (pContMsg->event == kContainAdd)
		{
			pARSrv->BeginContact(ObjId(), iPlayer);
		}
		else if (pContMsg->event == kContainRemove)
		{
			pARSrv->EndContact(ObjId(), iPlayer);
		}
	}

	return cBaseScript::OnContained(pContMsg, mpReply);
}


/***
 * TrapCreate
 */
int cScr_CreateTrap::LinkIter(ILinkSrv* pLS, ILinkQuery* pLQ, IScript* pScript, void*)
{
	cScr_CreateTrap* scrCreateTrap = static_cast<cScr_CreateTrap*>(pScript);
	SService<IObjectSrv> pOS(g_pScriptManager);
	sLink sl;
	pLQ->Link(&sl);
	pOS->Teleport(sl.dest, cScrVec::Zero, cScrVec::Zero, scrCreateTrap->ObjId());
	pLS->Destroy(pLQ->ID());
	return 1;
}

long cScr_CreateTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
		IterateLinks("Contains", ObjId(), 0, LinkIter, this, NULL);

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * WatchMe
 */
long cScr_WatchMe::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	char szSelf[12];
	sprintf(szSelf, "%d", ObjId());
	cAnsiStr strWatchers;
	char* pszParam = GetObjectParamString(ObjId(), "watcher");
	if (pszParam)
	{
		strWatchers = "@";
		strWatchers += pszParam;
		g_pMalloc->Free(pszParam);
	}
	else
		strWatchers = "@Human";

	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	pLS->CreateMany(pLTS->LinkKindNamed("AIWatchObj"), static_cast<const char*>(strWatchers), szSelf);

	return cBaseScript::OnBeginScript(pMsg, mpReply);
}


/***
 * WindowShade
 */
void cScr_WindowShade::InitModes(void)
{
	static const int _OnModes[] = { 0,1,2,4,4,4,6,6,8,9 };
	static const int _OffModes[] = {5,7,5,3,3,5,7,7,7,5 };

	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "AnimLight"))
	{
		cMultiParm mpMode;
		pPS->Get(mpMode, ObjId(), "AnimLight", "Mode");
		if (static_cast<int>(mpMode) >= 0 && static_cast<int>(mpMode) < 10)
		{
			m_iOnMode = _OnModes[static_cast<int>(mpMode)];
			m_iOffMode = _OffModes[static_cast<int>(mpMode)];
		}
		else
		{
			m_iOnMode = kAnimLight_MaxBrightness;
			m_iOffMode = kAnimLight_MinBrightness;
		}

		if (pPS->Possessed(ObjId(), "TerrRepOff")
		 && pPS->Possessed(ObjId(), "TerrRepOn"))
		{
			cMultiParm mpOld, mpNew;
			pPS->Get(mpOld, ObjId(), "TerrRepOff", NULL);
			pPS->Get(mpNew, ObjId(), "TerrRepOn", NULL);
			SService<IAnimTextureSrv> pTS(g_pScriptManager);
			if (static_cast<int>(mpMode) == m_iOnMode)
				pTS->ChangeTexture(ObjId(), NULL, mpOld, NULL, mpNew);
			else
				pTS->ChangeTexture(ObjId(), NULL, mpNew, NULL, mpOld);
		}
	}
	else
	{
		m_iOnMode = kAnimLight_MaxBrightness;
		m_iOffMode = kAnimLight_MinBrightness;
	}
}

void cScr_WindowShade::TurnLightOn(void)
{
	if (m_iOnMode < 0)
		InitModes();

	SService<ILightScrSrv> pLS(g_pScriptManager);
	pLS->SetMode(ObjId(), m_iOnMode);

	if (!m_bBroken)
	{
		SService<IPropertySrv> pPS(g_pScriptManager);
		if (pPS->Possessed(ObjId(), "TerrRepOff")
		 && pPS->Possessed(ObjId(), "TerrRepOn"))
		{
			cMultiParm mpOld, mpNew;
			pPS->Get(mpOld, ObjId(), "TerrRepOff", NULL);
			pPS->Get(mpNew, ObjId(), "TerrRepOn", NULL);
			SService<IAnimTextureSrv> pTS(g_pScriptManager);
			pTS->ChangeTexture(ObjId(), NULL, mpOld, NULL, mpNew);
		}
	}

	Trigger(true);
}

void cScr_WindowShade::TurnLightOff(void)
{
	if (m_iOffMode < 0)
		InitModes();

	SService<ILightScrSrv> pLS(g_pScriptManager);
	pLS->SetMode(ObjId(), m_iOffMode);

	if (!m_bBroken)
	{
		SService<IPropertySrv> pPS(g_pScriptManager);
		if (pPS->Possessed(ObjId(), "TerrRepOff")
		 && pPS->Possessed(ObjId(), "TerrRepOn"))
		{
			cMultiParm mpOld, mpNew;
			pPS->Get(mpNew, ObjId(), "TerrRepOff", NULL);
			pPS->Get(mpOld, ObjId(), "TerrRepOn", NULL);
			SService<IAnimTextureSrv> pTS(g_pScriptManager);
			pTS->ChangeTexture(ObjId(), NULL, mpOld, NULL, mpNew);
		}
	}

	Trigger(false);
}

void cScr_WindowShade::Trigger(bool bTurnOn)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	pLS->BroadcastOnAllLinks(ObjId(), (bTurnOn ? "TurnOn" : "TurnOff"), pLTS->LinkKindNamed("~ParticleAttachement"));
	pLS->BroadcastOnAllLinks(ObjId(), (bTurnOn ? "TurnOn" : "TurnOff"), pLTS->LinkKindNamed("ControlDevice"));
}

bool cScr_WindowShade::IsLightOn(void)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (!pPS->Possessed(ObjId(), "AnimLight"))
		return false;

	if (m_iOnMode < 0)
		return true;

	cMultiParm mpMode;
	pPS->Get(mpMode, ObjId(), "AnimLight", "Mode");
	return m_iOnMode == static_cast<int>(mpMode);
}

long cScr_WindowShade::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	m_iOnMode.Init(-1);
	m_iOffMode.Init(-1);
	m_bBroken.Init(0);

	return cBaseTrap::OnBeginScript(pMsg, mpReply);
}

long cScr_WindowShade::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
	if (pSimMsg->fStarting)
	{
		InitModes();
	}

	return cBaseTrap::OnSim(pSimMsg, mpReply);
}

long cScr_WindowShade::OnSlain(sSlayMsg* pSlayMsg, cMultiParm& mpReply)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "TerrRepOff")
	 && pPS->Possessed(ObjId(), "TerrRepOn")
	 && pPS->Possessed(ObjId(), "TerrRepDestroy"))
	{
		cMultiParm mpOld, mpNew;
		pPS->Get(mpOld, ObjId(), (IsLightOn())?"TerrRepOn":"TerrRepOff", NULL);
		pPS->Get(mpNew, ObjId(), "TerrRepDestroy", NULL);
		SService<IAnimTextureSrv> pTS(g_pScriptManager);
		pTS->ChangeTexture(ObjId(), NULL, mpOld, NULL, mpNew);
	}
	m_bBroken = 1;
	TurnLightOff();

	return cBaseTrap::OnSlain(pSlayMsg, mpReply);
}

long cScr_WindowShade::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "Toggle"))
	{
		if (IsLightOn())
			TurnLightOff();
		else
			TurnLightOn();
		return 0;
	}

	return cBaseTrap::OnMessage(pMsg, mpReply);
}

long cScr_WindowShade::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
		TurnLightOn();
	else
		TurnLightOff();

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

void cScr_WindowShadeRandom::SetAutoTimer(bool bChangeMode)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	SService<IDataSrv> pDS(g_pScriptManager);

	if (bChangeMode)
	{
		int iProb = 32;
		if (pPS->Possessed(ObjId(), "ScriptTiming"))
		{
			cMultiParm mpProb;
			pPS->Get(mpProb, ObjId(), "ScriptTiming", NULL);
			iProb = static_cast<int>(mpProb);
		}
		if (iProb > pDS->RandInt(0,99))
			TurnLightOn();
		else
			TurnLightOff();
	}

	int iMaxTime = 15000;
	int iMinTime = 5000;
	char* pszTime = GetObjectParamString(ObjId(), "auto_time_max");
	if (pszTime)
	{
		iMaxTime = strtotime(pszTime);
		iMinTime = GetObjectParamTime(ObjId(), "auto_time_min");
		if (iMinTime == 0)
			iMinTime = iMaxTime / 3;
		g_pMalloc->Free(pszTime);
	}
	int iTime = pDS->RandInt(iMinTime, iMaxTime);
	if (iTime < 1000)
		iTime = 1000;
	m_hTimer = SetTimedMessage("WindowShade", iTime, kSTM_OneShot);
}

long cScr_WindowShadeRandom::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	m_hTimer.Init();

	return cScr_WindowShade::OnBeginScript(pMsg, mpReply);
}

long cScr_WindowShadeRandom::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
	if (pSimMsg->fStarting)
	{
		SetAutoTimer(false);
	}

	return cScr_WindowShade::OnSim(pSimMsg, mpReply);
}

long cScr_WindowShadeRandom::OnSlain(sSlayMsg* pSlayMsg, cMultiParm& mpReply)
{
	if (m_hTimer)
	{
		KillTimedMessage(m_hTimer);
		m_hTimer = NULL;
	}

	return cScr_WindowShade::OnSlain(pSlayMsg, mpReply);
}

long cScr_WindowShadeRandom::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (!strcmp(pTimerMsg->name, "WindowShade"))
	{
		m_hTimer = NULL;
		SetAutoTimer(true);
		return 0;
	}

	return cScr_WindowShade::OnTimer(pTimerMsg, mpReply);
}

long cScr_WindowShadeRandom::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	SetTiming(0);
	return cScr_WindowShade::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * TrapSlayer
 */
int cScr_SlayTrap::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript* pScript, void* pData)
{
	cScr_SlayTrap* scrSlayTrap = static_cast<cScr_SlayTrap*>(pScript);
	int iDamageType = reinterpret_cast<int>(pData);
	SService<IPropertySrv> pPS(g_pScriptManager);
	SService<IDamageSrv> pDS(g_pScriptManager);
	SService<IObjectSrv> pOS(g_pScriptManager);
	sLink sl;
	pLQ->Link(&sl);
	if (pPS->Possessed(sl.dest, "HitPoints"))
	{
		pPS->SetSimple(sl.dest, "HitPoints", 1);
		pDS->Damage(sl.dest, scrSlayTrap->ObjId(), 1, iDamageType);
		true_bool __p;
		pOS->Exists(__p, sl.dest);
		// The damage model already killed the object. Don't try to slay it again.
		if (!__p) return 1;
	}
	pDS->Slay(sl.dest, scrSlayTrap->ObjId());

	return 1;
}

long cScr_SlayTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
	{
		SService<IObjectSrv> pOS(g_pScriptManager);
		object oDamageType = GetObjectParamObject(ObjId(), "damage");
		if (!oDamageType)
			pOS->Named(oDamageType, "BashStim");

		IterateLinks("ControlDevice", ObjId(), 0, LinkIter, this, reinterpret_cast<void*>(int(oDamageType)));

		pOS->Destroy(ObjId());
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * TrapTerminator
 */
int cScr_ReallyDestroy::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript*, void*)
{
	SService<IObjectSrv> pOS(g_pScriptManager);
	sLink sl;
	pLQ->Link(&sl);
	pOS->Destroy(sl.dest);

	return 1;
}

long cScr_ReallyDestroy::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
	{
		IterateLinks("ControlDevice", ObjId(), 0, LinkIter, this, NULL);

		SService<IObjectSrv> pOS(g_pScriptManager);
		pOS->Destroy(ObjId());
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * NewCorpseFrob
 */
long cScr_NewCorpseFrob::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	bool bIsDead = true;
	SService<IPropertySrv> pPS(g_pScriptManager);
	cMultiParm mpAIProp;
	if (pPS->Possessed(ObjId(), "AI"))
	{
		pPS->Get(mpAIProp, ObjId(), "AI", "Behavior set");
		if (_stricmp(static_cast<const char*>(mpAIProp), "null") != 0)
		{
			cMultiParm mpMode;
			if (pPS->Possessed(ObjId(), "AI_Mode"))
			{
				pPS->Get(mpMode, ObjId(), "AI_Mode", NULL);
				bIsDead = static_cast<int>(mpMode) == kAIM_Dead;
			}
			else
				bIsDead = false;
		}
	}
	if (bIsDead)
		RemoveSingleMetaProperty("FrobInert", ObjId());
	else
		AddSingleMetaProperty("FrobInert", ObjId());

	return cBaseAIScript::OnBeginScript(pMsg, mpReply);
}

long cScr_NewCorpseFrob::OnSlain(sSlayMsg* pSlayMsg, cMultiParm& mpReply)
{
	RemoveSingleMetaProperty("M-KnockedOut", ObjId());

	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "PhysType"))
	{
		pPS->Set(ObjId(), "PhysType", "Remove on Sleep", 1);
	}

	return cBaseAIScript::OnSlain(pSlayMsg, mpReply);
}

long cScr_NewCorpseFrob::OnAIModeChange(sAIModeChangeMsg* pAIModeMsg, cMultiParm& mpReply)
{
	if (pAIModeMsg->mode == kAIM_Dead)
	{
		RemoveSingleMetaProperty("FrobInert", ObjId());
	}
	else if (pAIModeMsg->previous_mode == kAIM_Dead)
	{
		AddSingleMetaProperty("FrobInert", ObjId());
	}

	return cBaseAIScript::OnAIModeChange(pAIModeMsg, mpReply);
}

long cScr_NewCorpseFrob::OnFrobWorldEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	SService<IDarkGameSrv> pGS(g_pScriptManager);
	if (pGS->BindingGetFloat("auto_search") != 0)
	{
		int iContents = 0;
		{
			SService<IPropertySrv> pPS(g_pScriptManager);
			SService<ILinkSrv> pLS(g_pScriptManager);
			SService<ILinkToolsSrv> pLTS(g_pScriptManager);
			linkset ls;
			pLS->GetAll(ls, pLTS->LinkKindNamed("Contains"), ObjId(), 0);
			for (; ls.AnyLinksLeft(); ls.NextLink())
			{
				if (*reinterpret_cast<int*>(ls.Data()) < 0)
				{
					// At this point, CorpseFrobHack just returns
					// but this has caused problems, so we go the extra
					// step of checking the frob info.
					sLink sl = ls.Get();
					if (pPS->Possessed(sl.dest, "FrobInfo"))
					{
						cMultiParm mpFrob;
						pPS->Get(mpFrob, sl.dest, "FrobInfo", "World Action");
						if (static_cast<int>(mpFrob) & 0x1)
						{
							iContents = sl.dest;
							break;
						}
					}
				}
			}
		}

		if (iContents != 0)
		{
			SService<IContainSrv> pCS(g_pScriptManager);
			pCS->Add(iContents, pFrobMsg->Frobber, 0, 1);
			mpReply = 0;
		}
	}

	return cBaseAIScript::OnFrobWorldEnd(pFrobMsg, mpReply);
}


/***
 * FireShadowEcology
 */
void cScr_SpawnEcology::MakeFirer(object iSpawnObj)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	link lFirer;
	pLS->Create(lFirer, pLTS->LinkKindNamed("Firer"), iSpawnObj, ObjId());
}

void cScr_SpawnEcology::AttemptSpawn(void)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	true_bool bFirer;
	pLS->AnyExist(bFirer, pLTS->LinkKindNamed("Firer"), 0, ObjId());
	if (bFirer)
		return;
	linkset ls;
	pLS->GetAllInheritedSingle(ls, pLTS->LinkKindNamed("ControlDevice"), ObjId(), 0);
	if (ls.AnyLinksLeft())
	{
		SService<IObjectSrv> pOS(g_pScriptManager);
		sLink sl = ls.Get();
		object oSpawn;
		pOS->BeginCreate(oSpawn, sl.dest);
		if (oSpawn)
		{
			pOS->Teleport(oSpawn, cScrVec::Zero, cScrVec::Zero, ObjId());
			pOS->EndCreate(oSpawn);
			SetTimedMessage("Firer", 100, kSTM_OneShot, oSpawn);
			SService<ISoundScrSrv> pSounds(g_pScriptManager);
			true_bool __p;
			pSounds->PlayEnvSchema(__p, ObjId(), "Event Activate", ObjId(), oSpawn, kEnvSoundAtObjLoc, kSoundNetNormal);
			CDSend("TurnOn", ObjId(), oSpawn);
		}
	}
}

long cScr_SpawnEcology::OnTweqComplete(sTweqMsg* pTweqMsg, cMultiParm& mpReply)
{
	if (pTweqMsg->Type == kTweqTypeFlicker && pTweqMsg->Op == kTweqOpFrameEvent)
	{
		AttemptSpawn();
	}

	return cBaseScript::OnTweqComplete(pTweqMsg, mpReply);
}

long cScr_SpawnEcology::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (!strcmp(pTimerMsg->name, "Firer"))
	{
		MakeFirer(pTimerMsg->data);
		return 0;
	}

	return cBaseScript::OnTimer(pTimerMsg, mpReply);
}


/***
 * FireShadowFlee
 */
long cScr_CorpseFlee::OnSlain(sSlayMsg* pSlayMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	linkset ls;
	pLS->GetAllInheritedSingle(ls, pLTS->LinkKindNamed("CorpsePart"), ObjId(), 0);
	for (; ls.AnyLinksLeft(); ls.NextLink())
	{
		sLink sl = ls.Get();
		object oDrop;
		pPhys->LaunchProjectile(oDrop, ObjId(), sl.dest, 0, 10, cScrVec::Zero);
	}

	SService<IObjectSrv> pOS(g_pScriptManager);
	SService<IPropertySrv> pPS(g_pScriptManager);
	object oMP;
	pOS->Named(oMP, "M-FireShadowFlee");
	if (oMP)
		pOS->AddMetaProperty(ObjId(), oMP);
	pPS->Add(ObjId(), "TimeWarp");
	pPS->SetSimple(ObjId(), "TimeWarp", 13.0f/16.0f);

	SetTimedMessage("CorpseFlee", 1000, kSTM_Periodic);

	return cBaseScript::OnSlain(pSlayMsg, mpReply);
}

long cScr_CorpseFlee::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (!strcmp(pTimerMsg->name, "CorpseFlee"))
	{
		SService<IObjectSrv> pOS(g_pScriptManager);
		SService<IPropertySrv> pPS(g_pScriptManager);
		cMultiParm mpProp;
		pPS->Get(mpProp, ObjId(), "TimeWarp", NULL);
		double fSpeed = mpProp;
		fSpeed *= 0.8125;
		true_bool bVis;
		if (fSpeed < 0.03 || ! *pOS->RenderedThisFrame(bVis, ObjId()))
		{
			pOS->Destroy(ObjId());
		}
		else
		{
			mpProp = fSpeed;
			pPS->SetSimple(ObjId(), "TimeWarp", mpProp);
		}
		return 0;
	}

	return cBaseScript::OnTimer(pTimerMsg, mpReply);
}


/***
 * ReallyLocked
 */
long cScr_LockFrobInert::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
	if (pSimMsg->fStarting)
	{
		SService<ILockSrv> pLock(g_pScriptManager);
		if (pLock->IsLocked(ObjId()))
			AddSingleMetaProperty("FrobInert", ObjId());
		else
			RemoveSingleMetaProperty("FrobInert", ObjId());
	}

	return cBaseScript::OnSim(pSimMsg, mpReply);
}

long cScr_LockFrobInert::OnNowLocked(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (IsSim())
		AddSingleMetaProperty("FrobInert", ObjId());

	return cBaseScript::OnNowLocked(pMsg, mpReply);
}

long cScr_LockFrobInert::OnNowUnlocked(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (IsSim())
		RemoveSingleMetaProperty("FrobInert", ObjId());

	return cBaseScript::OnNowUnlocked(pMsg, mpReply);
}


/***
 * FactoryBase
 */
cScr_Factory::cScr_Factory(const char* pszName, int iHostObjId)
	: cBaseScript(pszName, iHostObjId)
{
	static const sMessageHandler handlers[] = 
		{{"Report",HandleReport}, {"SynchUp",HandleSynchUp}, {"Target?",HandleTarget}};
	RegisterMessageHandlers(handlers, 3);
}

long cScr_Factory::HandleReport(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cScr_Factory*>(pScript)->OnReport(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cScr_Factory::HandleSynchUp(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cScr_Factory*>(pScript)->OnSynchUp(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cScr_Factory::HandleTarget(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cScr_Factory*>(pScript)->OnTarget(pMsg, static_cast<cMultiParm&>(*pReply));
}

bool cScr_Factory::IsLocked(void)
{
	SService<ILockSrv> pLock(g_pScriptManager);
	return pLock->IsLocked(ObjId());
}

void cScr_Factory::DoTrigger(bool bTurnOn)
{
	bool bInvert = false;
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "TrapFlags"))
	{
		cMultiParm mpTrapFlags;
		pPS->Get(mpTrapFlags, ObjId(), "TrapFlags", NULL);
		bInvert = static_cast<int>(mpTrapFlags) & kTrapFlagInvert;
	}

	CDSend((bTurnOn ^ bInvert) ? "TurnOn" : "TurnOff", ObjId());
}

int cScr_Factory::TriggerIter(ILinkSrv*, ILinkQuery* pLQ, IScript* pScript, void* pData)
{
	cScr_Factory* pScrFactory = static_cast<cScr_Factory*>(pScript);
	const char* pszMsg = reinterpret_cast<const char*>(pData);
	sLink sl;
	pLQ->Link(&sl);
	pScrFactory->PostMessage(sl.dest, pszMsg);
	return 1;
}

void cScr_Factory::DoTrigger(bool bTurnOn, bool bReverse, const char* pszParam)
{
	DoTrigger(bTurnOn ? "TurnOn" : "TurnOff", bReverse, pszParam);
}

void cScr_Factory::DoTrigger(const char* pszMsg, bool bReverse, const char* pszParam)
{
	IterateLinksByData(bReverse ? "~ScriptParams" : "ScriptParams", ObjId(), 0,
			pszParam, -1, TriggerIter, this, reinterpret_cast<void*>(const_cast<char*>(pszMsg)));
}

cScr_Factory::eLeverDirection cScr_Factory::NextDirection(void)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (! pPS->Possessed(ObjId(), "StTweqJoints"))
		return kLeverDirectionForward;
	cMultiParm mpState;
	pPS->Get(mpState, ObjId(), "StTweqJoints", "AnimS");
	return (static_cast<int>(mpState) & kTweqFlagReverse) ?
			kLeverDirectionBackward : kLeverDirectionForward;
}

void cScr_Factory::DoGoForward(void)
{
	BeginGoForward();

	SService<IActReactSrv> pARSrv(g_pScriptManager);
	pARSrv->React(pARSrv->GetReactionNamed("tweq_control"), 1.0f, ObjId(), 0,
		kTweqTypeJoints, kTweqDoForward,
		cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef);

	PlayEnvSchema(ObjId(), "Event StateChange, DirectionState Forward", ObjId(), 0, kEnvSoundOnObj, kSoundNetNormal);

	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	pLS->BroadcastOnAllLinks(ObjId(), "SynchUp", pLTS->LinkKindNamed("~FrobProxy"));
}

void cScr_Factory::DoGoBackward(void)
{
	BeginGoBackward();

	SService<IActReactSrv> pARSrv(g_pScriptManager);
	pARSrv->React(pARSrv->GetReactionNamed("tweq_control"), 1.0f, ObjId(), 0,
		kTweqTypeJoints, kTweqDoReverse,
		cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef);

	PlayEnvSchema(ObjId(), "Event StateChange, DirectionState Reverse", ObjId(), 0, kEnvSoundOnObj, kSoundNetNormal);

	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	pLS->BroadcastOnAllLinks(ObjId(), "SynchUp", pLTS->LinkKindNamed("~FrobProxy"));
}

void cScr_Factory::DoSetState(eLeverDirection dir)
{
	if (dir == kLeverDirectionBackward)
		DoGoBackward();
	else
		DoGoForward();
}

void cScr_Factory::DoReport(void)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	pLS->BroadcastOnAllLinksData(ObjId(), "Report", pLTS->LinkKindNamed("~Owns"), ReportType());
}

char const * cScr_Factory::ReportType(void)
{
	return "Base";
}

long cScr_Factory::OnTweqComplete(sTweqMsg* pTweqMsg, cMultiParm& mpReply)
{
	if (pTweqMsg->Type == kTweqTypeJoints && pTweqMsg->Op == kTweqOpHaltTweq)
	{
		if (pTweqMsg->Dir == kTweqDirForward)
			EndGoForward();
		else
			EndGoBackward();
	}

	return cBaseScript::OnTweqComplete(pTweqMsg, mpReply);
}

long cScr_Factory::OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply)
{
	DoGoForward();

	return cBaseScript::OnTurnOn(pMsg, mpReply);
}

long cScr_Factory::OnTurnOff(sScrMsg* pMsg, cMultiParm& mpReply)
{
	DoGoBackward();

	return cBaseScript::OnTurnOff(pMsg, mpReply);
}

long cScr_Factory::OnReport(sScrMsg*, cMultiParm&)
{
	DoReport();
	return 0;
}

long cScr_Factory::OnSynchUp(sScrMsg* pMsg, cMultiParm&)
{
	cMultiParm mpTarget = 0;
	SendMessage(mpTarget, pMsg->from, "Target?");
	DoSetState(eLeverDirection(static_cast<int>(mpTarget)));

	return 0;
}

long cScr_Factory::OnTarget(sScrMsg*, cMultiParm& mpReply)
{
	mpReply = int(NextDirection());
	return 0;
}


/***
 * FactoryLever
 */
char const * cScr_FactoryLever::ReportType(void)
{
	return "Lever";
}

void cScr_FactoryLever::BeginGoForward(void)
{
	DoTrigger(true, false, "LockMe");
}

void cScr_FactoryLever::EndGoForward(void)
{
	DoTrigger(true);
	DoReport();
}

void cScr_FactoryLever::EndGoBackward(void)
{
	DoTrigger(false);
	DoTrigger(false, false, "LockMe");
	DoReport();
}

long cScr_FactoryLever::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	DoTrigger(NextDirection() == kLeverDirectionBackward, false, "LockMe");

	return cScr_Factory::OnBeginScript(pMsg, mpReply);
}

long cScr_FactoryLever::OnFrobWorldEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	if (IsLocked())
	{
		DoTrigger(true, false, "ErrorOutput");
	}
	else
	{
		if (NextDirection() == kLeverDirectionBackward)
			DoGoBackward();
		else
			DoGoForward();
	}

	return cScr_Factory::OnFrobWorldEnd(pFrobMsg, mpReply);
}


/***
 * FactoryMold
 */
char const * cScr_FactoryMold::ReportType(void)
{
	return "Mold";
}

int cScr_FactoryMold::ProductOwnerIter(ILinkSrv* pLS, ILinkQuery* pLQ, IScript*, void* pData)
{
	object iProduct = reinterpret_cast<int>(pData);
	sLink sl;
	pLQ->Link(&sl);
	link lOwns;
	pLS->Create(lOwns, sl.flavor, iProduct, sl.dest);
	return 1;
}

void cScr_FactoryMold::BeginGoForward(void)
{
	DoTrigger(false, true, "Mold");

	if (m_iState != 0)
	{
		sLink slProduct;
		if (GetOneLinkInheritedSrc("Contains", ObjId(), 0, &slProduct))
		{
			SService<IObjectSrv> pOS(g_pScriptManager);
			SService<IPropertySrv> pPS(g_pScriptManager);

			object iProduct;
			pOS->BeginCreate(iProduct, slProduct.dest);
			if (iProduct)
			{
				pPS->SetSimple(iProduct, "HasRefs", 1);
				if (pPS->Possessed(iProduct, "PhysType"))
					pPS->Remove(iProduct, "PhysType");
				pOS->EndCreate(iProduct);

				cScrVec vOffset;
				if (pPS->Possessed(ObjId(), "PhysAttr"))
				{
					cMultiParm mpProp;
					pPS->Get(mpProp, ObjId(), "PhysAttr", "COG Offset");
					vOffset = *static_cast<const mxs_vector*>(mpProp);
				}
				pOS->Teleport(iProduct, vOffset, cScrVec::Zero, ObjId());

				IterateLinks("~Owns", ObjId(), 0, ProductOwnerIter, this,
						reinterpret_cast<void*>(static_cast<int>(iProduct)));

				SService<ILinkSrv> pLS(g_pScriptManager);
				SService<ILinkToolsSrv> pLTS(g_pScriptManager);
				link lProxy;
				pLS->Create(lProxy, pLTS->LinkKindNamed("~FrobProxy"), ObjId(), iProduct);
				pLTS->LinkSetData(lProxy, "FrobProxyMask", 0x10);
				pPS->SetSimple(iProduct, "PickBias", 5.0);

				DoTrigger(true);
			}
		}

		m_iState = 0;
	}

	cScr_FactoryLever::BeginGoForward();
}

void cScr_FactoryMold::EndGoForward(void)
{
	DoTrigger(true, false, "LockMe");
}

void cScr_FactoryMold::BeginGoBackward(void)
{
	SService<IObjectSrv> pOS(g_pScriptManager);
	object iProduct = GetOneLinkDest("~FrobProxy", ObjId());
	if (iProduct)
	{
		pOS->Destroy(iProduct);
		m_iState = 1;
	}

	cScr_FactoryLever::BeginGoBackward();
}

void cScr_FactoryMold::EndGoBackward(void)
{
	DoTrigger(false);
	DoTrigger(false, false, "LockMe");
}

long cScr_FactoryMold::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	m_iState.Init(0);

	return cScr_FactoryLever::OnBeginScript(pMsg, mpReply);
}

long cScr_FactoryMold::OnFrobToolEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	cMultiParm mpIgnore;
	SendMessage(mpIgnore, pFrobMsg->DstObjId, "SocketMe");
	DoTrigger(NextDirection() == kLeverDirectionBackward, false, "LockMe");

	return cScr_FactoryLever::OnFrobToolEnd(pFrobMsg, mpReply);
}

long cScr_FactoryMold::OnFrobWorldEnd(sFrobMsg*, cMultiParm&)
{
	return 0;
}

long cScr_FactoryMold::OnFrobWorldBegin(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	true_bool bHasProduct;
	pLS->AnyExist(bHasProduct, pLTS->LinkKindNamed("~FrobProxy"), ObjId(), 0);
	if (bHasProduct)
	{
		link lProxy;
		sLink sl;
		pLS->GetOne(lProxy, pLTS->LinkKindNamed("~FrobProxy"), ObjId(), 0);
		pLTS->LinkGet(lProxy, sl);
		pLS->Destroy(lProxy);

		object iProduct = sl.dest;

		link lOwns;
		linkset lsOwns;
		pLS->GetAll(lsOwns, pLTS->LinkKindNamed("~Owns"), iProduct, 0);
		for (; lsOwns.AnyLinksLeft(); lsOwns.NextLink())
		{
			pLS->Destroy(lsOwns.Link());
		}
		pLS->Create(lOwns, pLTS->LinkKindNamed("~Owns"), iProduct, pFrobMsg->Frobber);

		SService<IPropertySrv> pPS(g_pScriptManager);
		pPS->Remove(iProduct, "PickBias");

		SService<IContainSrv> pCS(g_pScriptManager);
		pCS->Add(iProduct, pFrobMsg->Frobber, 0, 1);

		mpReply = 0;
	}

	return 0;
}

long cScr_FactoryMold::OnContained(sContainedScrMsg* pContMsg, cMultiParm& mpReply)
{
	if (pContMsg->event == kContainAdd)
	{
		SService<ILinkSrv> pLS(g_pScriptManager);
		SService<ILinkToolsSrv> pLTS(g_pScriptManager);
		pLS->BroadcastOnAllLinks(ObjId(), "UnsocketMe", pLTS->LinkKindNamed("Owns"));
	}

	return cScr_FactoryLever::OnContained(pContMsg, mpReply);
}

long cScr_FactoryMold::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "Full"))
	{
		m_iState = 1;
		DoReport();
		return 0;
	}
	if (!_stricmp(pMsg->message, "Full?"))
	{
		mpReply = int(m_iState == 1);
		return 0;
	}

	return cScr_FactoryLever::OnMessage(pMsg, mpReply);
}


/***
 * MoldSocket
 */
static bool LinkParam(const char* pszParams, char* pszName, char* pszData)
{
	if (!pszParams)
		return false;
	if (!strnicmp(pszParams, "SP:", 3))
	{
		strcpy(pszName, "ScriptParams");
		char const* p = pszParams + 3;
		while (*p == ' ') ++p;
		strncpy(pszData, p, 15);
		return true;
	}
	else if (!strnicmp(pszParams, "~SP:", 4))
	{
		strcpy(pszName, "~ScriptParams");
		char const* p = pszParams + 4;
		while (*p == ' ') ++p;
		strncpy(pszData, p, 15);
		return true;
	}
	else if (strlen(pszParams) > 0)
	{
		strncpy(pszName, pszParams, 15);
		pszData[0] = '\0';
		return true;
	}
	return false;
}

long cScr_MoldSocket::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "SocketMe"))
	{
		{
			SService<IPropertySrv> pPS(g_pScriptManager);
			SService<IKeySrv> pKeys(g_pScriptManager);
			if (pPS->Possessed(ObjId(), "KeyDst")
			 && !pKeys->TryToUseKey(pMsg->from, ObjId(), kKeyUseCheck))
			{
				mpReply = 0;
				return 0;
			}
		}

		SService<ILinkSrv> pLS(g_pScriptManager);
		SService<ILinkToolsSrv> pLTS(g_pScriptManager);

		{
			SService<IContainSrv> pCont(g_pScriptManager);
			pCont->Remove(pMsg->from, 0);
		}
		{
			SService<IObjectSrv> pOS(g_pScriptManager);
			SService<IPropertySrv> pPS(g_pScriptManager);
			cScrVec vOffset;
			if (pPS->Possessed(ObjId(), "PhysAttr"))
			{
				cMultiParm mpProp;
				pPS->Get(mpProp, ObjId(), "PhysAttr", "COG Offset");
				vOffset = *static_cast<const mxs_vector*>(mpProp);
			}
			pOS->Teleport(pMsg->from, vOffset, cScrVec::Zero, ObjId());
		}

		char szLinkName[16]; // string length restricted by ScriptParams
		char szLinkData[16];
		linkset lsSParams;
		pLS->GetAll(lsSParams, pLTS->LinkKindNamed("ScriptParams"), ObjId(), 0);
		for (; lsSParams.AnyLinksLeft(); lsSParams.NextLink())
		{
			sLink sl = lsSParams.Get();
			if (LinkParam(reinterpret_cast<const char*>(lsSParams.Data()), szLinkName, szLinkData))
			{
				linkkind lk = pLTS->LinkKindNamed(szLinkName);
				if (lk)
				{
					link l;
					pLS->Create(l, lk, pMsg->from, sl.dest);
					if (szLinkData[0] != '\0')
						pLTS->LinkSetData(l, NULL, szLinkData);
				}
			}
		}

		link lOwns;
		pLS->Create(lOwns, pLTS->LinkKindNamed("Owns"), pMsg->from, ObjId());

		AddSingleMetaProperty("FrobInert", ObjId());

		mpReply = 1;
		return 0;
	}
	if (!_stricmp(pMsg->message, "UnsocketMe"))
	{
		SService<ILinkSrv> pLS(g_pScriptManager);
		SService<ILinkToolsSrv> pLTS(g_pScriptManager);
		{
			cDynArray<object> aDestObjs;
			aDestObjs.resize(8);
			linkset lsSParams;
			pLS->GetAll(lsSParams, pLTS->LinkKindNamed("ScriptParams"), ObjId(), 0);
			for (; lsSParams.AnyLinksLeft(); lsSParams.NextLink())
			{
				sLink sl = lsSParams.Get();
				aDestObjs.append(sl.dest);
			}

			for (ulong i = 0; i < aDestObjs.size(); ++i)
			{
				linkset lsLinks;
				pLS->GetAll(lsLinks, 0, pMsg->from, aDestObjs[i]);
				for (; lsLinks.AnyLinksLeft(); lsLinks.NextLink())
				{
					pLS->Destroy(lsLinks.Link());
				}
			}
		}

		linkset lsOwns;
		pLS->GetAll(lsOwns, pLTS->LinkKindNamed("~Owns"), ObjId(), 0);
		for (; lsOwns.AnyLinksLeft(); lsOwns.NextLink())
		{
			pLS->Destroy(lsOwns.Link());
		}

		RemoveSingleMetaProperty("FrobInert", ObjId());

		mpReply = 1;
		return 0;
	}

	return cBaseScript::OnMessage(pMsg, mpReply);
}


/***
 * FactoryCauldron
 */
char const * cScr_FactoryCauldron::ReportType(void)
{
	return "Cauld";
}

void cScr_FactoryCauldron::BeginGoForward(void)
{
	SService<ISoundScrSrv> pSounds(g_pScriptManager);

	true_bool __p;
	pSounds->HaltSchema(__p, ObjId(), "cauldron_lp", 0);
	DoTrigger("GoForward", false, "Synch");
	pSounds->PlaySchemaAtObject(__p, ObjId(), StrToObject("cauldron_pivot"), ObjId(), kSoundNetNormal);
}

void cScr_FactoryCauldron::EndGoForward(void)
{
	SService<ISoundScrSrv> pSounds(g_pScriptManager);
	true_bool __p;
	pSounds->HaltSchema(__p, ObjId(), "cauldron_pivot", 0);

	DoTrigger(false, false, "Synch");
	DoTrigger(true, false, "Sparks");

	SService<IPropertySrv> pPS(g_pScriptManager);
	pPS->Remove(ObjId(), "CfgTweqModels");
	SService<IActReactSrv> pARSrv(g_pScriptManager);
	pARSrv->React(pARSrv->GetReactionNamed("tweq_control"), 1.0f, ObjId(), 0,
		kTweqTypeModels, kTweqDoForward,
		cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef);
	SService<ILightScrSrv> pLights(g_pScriptManager);
	pLights->SetMode(ObjId(), kAnimLight_Random);

	DoTrigger(true);
	pSounds->PlaySchemaAtObject(__p, ObjId(), StrToObject("lava_pour"), ObjId(), kSoundNetNormal);

	DoReport();
}

void cScr_FactoryCauldron::BeginGoBackward(void)
{
	SService<ISoundScrSrv> pSounds(g_pScriptManager);

	true_bool __p;
	pSounds->HaltSchema(__p, ObjId(), "lava_pour", 0);
	DoTrigger(false, false, "Sparks");

	SService<IPropertySrv> pPS(g_pScriptManager);
	pPS->Set(ObjId(), "CfgTweqModels", "Rate", 1);
	SService<IActReactSrv> pARSrv(g_pScriptManager);
	pARSrv->React(pARSrv->GetReactionNamed("tweq_control"), 1.0f, ObjId(), 0,
		kTweqTypeModels, kTweqDoReverse,
		cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef);
	SService<ILightScrSrv> pLights(g_pScriptManager);
	pLights->SetMode(ObjId(), kAnimLight_MinBrightness);

	CDSend("Halt", ObjId());
	DoTrigger("GoReverse", false, "Synch");
	pSounds->PlaySchemaAtObject(__p, ObjId(), StrToObject("cauldron_pivot"), ObjId(), kSoundNetNormal);
}

void cScr_FactoryCauldron::EndGoBackward(void)
{
	SService<ISoundScrSrv> pSounds(g_pScriptManager);
	true_bool __p;
	pSounds->HaltSchema(__p, ObjId(), "cauldron_pivot", 0);
	DoTrigger(false, false, "Synch");
	pSounds->PlaySchemaAtObject(__p, ObjId(), StrToObject("cauldron_lp"), ObjId(), kSoundNetNormal);

	DoReport();
}

long cScr_FactoryCauldron::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
	if (pSimMsg->fStarting)
	{
		SService<ISoundScrSrv> pSounds(g_pScriptManager);
		true_bool __p;
		pSounds->PlaySchemaAtObject(__p, ObjId(), StrToObject("cauldron_lp"), ObjId(), kSoundNetNormal);
	}

	return cScr_Factory::OnSim(pSimMsg, mpReply);
}


/***
 * FactoryGauge
 */
char const * cScr_FactoryGauge::ReportType(void)
{
	return "Gauge";
}

float cScr_FactoryGauge::GetRate(int state)
{
	static const float _factory_rates[3] = { 1.75, 0.5, 30.0 };
	char* pszRates = GetObjectParamString(ObjId(), "rates");
	if (! pszRates)
	{
		return _factory_rates[state];
	}

	float rate = 0.0;
	float rates[3];
	switch (sscanf(pszRates, "%f , %f , %f", &rates[0], &rates[1], &rates[2]))
	{
		case 3:
			rate = rates[state];
			break;
		case 2:
			rate = (state == 2) ? rates[1] : rates[0];
			break;
		case 1:
			rate = rates[0];
			break;
		default:
			break;
	}

	g_pMalloc->Free(pszRates);
	return rate;
}

void cScr_FactoryGauge::BeginGoForward(void)
{
	if (m_iState == 2)
	{
		EndGoBackward();
	}
	object iMold = GetOneLinkByDataDest("ScriptParams", ObjId(), "Mold", -1);
	if (iMold)
	{
		cMultiParm mpReply = 0;
		SendMessage(mpReply, iMold, "Full?");
		if (static_cast<bool>(mpReply))
		{
			EndGoForward();
		}
	}
}

void cScr_FactoryGauge::EndGoForward(void)
{
	if (m_iState == 0)
	{
		DoTrigger("Full", false, "Mold");

		float overflow = GetObjectParamFloat(ObjId(), "overflow", 20.0);
		if (overflow > 0.0)
		{
			SService<IPropertySrv> pPS(g_pScriptManager);
			cMultiParm mpProp;
			cScrVec vJointCfg;
			if (pPS->Possessed(ObjId(), "CfgTweqJoints"))
			{
				pPS->Get(mpProp, ObjId(), "CfgTweqJoints", "    rate-low-high");
				vJointCfg.y = static_cast<const mxs_vector*>(mpProp)->y;
				vJointCfg.z = static_cast<const mxs_vector*>(mpProp)->z + overflow;
				vJointCfg.x = GetRate(1);
				mpProp = vJointCfg;
				pPS->Set(ObjId(), "CfgTweqJoints", "    rate-low-high", mpProp);
			}
			SService<IActReactSrv> pARSrv(g_pScriptManager);
			pARSrv->React(pARSrv->GetReactionNamed("tweq_control"), 1.0f, ObjId(), 0,
				kTweqTypeJoints, kTweqDoForward,
				cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef);
		}
		else
		{
			DoTrigger(true);
		}

		m_iState = 1;

	}
	else
	{
		DoTrigger(true);
	}
}

void cScr_FactoryGauge::BeginGoBackward(void)
{
	if (m_iState != 2)
	{
		SService<IPropertySrv> pPS(g_pScriptManager);
		cMultiParm mpProp;
		cScrVec vJointCfg;
		if (pPS->Possessed(ObjId(), "CfgTweqJoints"))
		{
			pPS->Get(mpProp, ObjId(), "CfgTweqJoints", "    rate-low-high");
			vJointCfg.y = static_cast<const mxs_vector*>(mpProp)->y;
			vJointCfg.z = static_cast<const mxs_vector*>(mpProp)->z;
			vJointCfg.x = GetRate(2);
			mpProp = vJointCfg;
			pPS->Set(ObjId(), "CfgTweqJoints", "    rate-low-high", mpProp);
		}

		m_iState = 2;
	}
}

void cScr_FactoryGauge::EndGoBackward(void)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	cMultiParm mpProp;
	cScrVec vJointCfg;
	if (pPS->Possessed(ObjId(), "CfgTweqJoints"))
	{
		pPS->Get(mpProp, ObjId(), "CfgTweqJoints", "    rate-low-high");
		vJointCfg.y = static_cast<const mxs_vector*>(mpProp)->y;
		vJointCfg.z = m_fMidJoint;
		vJointCfg.x = GetRate(0);
		mpProp = vJointCfg;
		pPS->Set(ObjId(), "CfgTweqJoints", "    rate-low-high", mpProp);
		pPS->Set(ObjId(), "JointPos", "Joint 1", vJointCfg.y);
	}

	m_iState = 0;
}

long cScr_FactoryGauge::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	m_iState.Init(0);

	if (!m_fMidJoint.Valid())
	{
		SService<IPropertySrv> pPS(g_pScriptManager);
		cMultiParm mpJointCfg;
		if (pPS->Possessed(ObjId(), "CfgTweqJoints"))
		{
			pPS->Get(mpJointCfg, ObjId(), "CfgTweqJoints", "    rate-low-high");
			m_fMidJoint = static_cast<const mxs_vector*>(mpJointCfg)->z;
		}
	}

	return cScr_Factory::OnBeginScript(pMsg, mpReply);
}

long cScr_FactoryGauge::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "Halt"))
	{
		SService<IActReactSrv> pARSrv(g_pScriptManager);
		pARSrv->React(pARSrv->GetReactionNamed("tweq_control"), 1.0f, ObjId(), 0,
			kTweqTypeJoints, kTweqDoHalt,
			cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef);
		return 0;
	}

	return cScr_Factory::OnMessage(pMsg, mpReply);
}


/***
 * FactoryLight
 */
long cScr_LightBlinker::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	m_hTimer.Init();

	return cBaseScript::OnBeginScript(pMsg, mpReply);
}

long cScr_LightBlinker::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (! strcmp(pTimerMsg->name, "Blink"))
	{
		m_hTimer = NULL;
		SimpleSend(ObjId(), ObjId(), "TurnOff");
		return 0;
	}

	return cBaseScript::OnTimer(pTimerMsg, mpReply);
}

long cScr_LightBlinker::OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPropertySrv> pPS(g_pScriptManager);

	// Let the AnimLight script handle things
	if (! pPS->Possessed(ObjId(), "AnimLight"))
	{
		pPS->SetSimple(ObjId(), "RenderType", 2);
	}
	PlayEnvSchema(ObjId(), "Event Activate", ObjId(), 0, kEnvSoundOnObj, kSoundNetNormal);

	int iTime = 550;
	if (pPS->Possessed(ObjId(), "ScriptTiming"))
	{
		cMultiParm mpTiming;
		pPS->Get(mpTiming, ObjId(), "ScriptTiming", NULL);
		iTime = static_cast<int>(mpTiming);
	}
	if (m_hTimer)
		KillTimedMessage(m_hTimer);
	m_hTimer = SetTimedMessage("Blink", iTime, kSTM_OneShot);

	return cBaseScript::OnTurnOn(pMsg, mpReply);
}

long cScr_LightBlinker::OnTurnOff(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (m_hTimer)
	{
		KillTimedMessage(m_hTimer);
		m_hTimer = NULL;
	}
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (! pPS->Possessed(ObjId(), "AnimLight"))
	{
		pPS->SetSimple(ObjId(), "RenderType", 0);
	}

	return cBaseScript::OnTurnOff(pMsg, mpReply);
}


/***
 * FactoryWork
 */
object cScr_FactoryWork::GetPath(void)
{
	object iHome = GetOwned(StrToObject("TrolPt"));
	return GetOneLinkDest("Route", iHome);
}

object cScr_FactoryWork::GetOwned(object iArchetype)
{
	if (!iArchetype)
		return 0;
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	SService<IObjectSrv> pOS(g_pScriptManager);

	linkset lsOwns;
	pLS->GetAll(lsOwns, pLTS->LinkKindNamed("Owns"), ObjId(), 0);
	for (; lsOwns.AnyLinksLeft(); lsOwns.NextLink())
	{
		sLink sl = lsOwns.Get();
		true_bool bRelated;
		pOS->InheritsFrom(bRelated, sl.dest, iArchetype);
		if (bRelated)
			return sl.dest;
	}
	return 0;
}

object cScr_FactoryWork::WhatControls(object iDevice)
{
	return iDevice ? GetOneLinkDest("~ControlDevice", iDevice) : 0;
}

bool cScr_FactoryWork::IsLeverOn(object iDevice)
{
	cMultiParm mpNextState = 1;
	SendMessage(mpNextState, iDevice, "Target?");
	return static_cast<int>(mpNextState) == 0;
}

bool cScr_FactoryWork::IsMoldFull(object iDevice)
{
	cMultiParm mpReply = 0;
	SendMessage(mpReply, iDevice, "Full?");
	return static_cast<bool>(mpReply);
}

bool cScr_FactoryWork::IsContained(object iHammer)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	true_bool bContained;
	pLS->AnyExist(bContained, pLTS->LinkKindNamed("Contains"), ObjId(), iHammer);
	return bContained;
}

bool cScr_FactoryWork::IsLocked(object iDevice)
{
	SService<ILockSrv> pLS(g_pScriptManager);
	return pLS->IsLocked(iDevice);
}

void cScr_FactoryWork::GotoWayPt(object iTarget, eAIActionPriority iSpeed)
{
	SService<IAIScrSrv> pAISrv(g_pScriptManager);
	true_bool __p;
	if (m_iWayPtDest != iTarget)
	{
		pAISrv->MakeGotoObjLoc(__p, ObjId(), GetOwned(StrToObject("TrolPt")),
						eAIScriptSpeed(iSpeed), iSpeed, cMultiParm::Undef);
		m_iWayPtDest = iTarget;
	}
	else
	{
		pAISrv->MakeGotoObjLoc(__p, ObjId(), iTarget,
						eAIScriptSpeed(iSpeed), iSpeed, cMultiParm::Undef);
	}
}

static const char* g_pszDefaultFrobMotion = "BH114310";
static const char* g_pszDefaultPickupMotion = "BH114605";
static const char* g_pszDefaultDropMotion = "BH214101";

void cScr_FactoryWork::PlayMotion(const char* pszType, const char* pszDefault)
{
	true_bool __p;
	char *pszMotion = GetObjectParamString(ObjId(), pszType);
	if (! pszMotion)
	{
		SInterface<ITraitManager> pTM(g_pScriptManager);
		object iArc = pTM->GetArchetype(ObjId());
		pszMotion = GetObjectParamString(iArc, pszType);
		if (! pszMotion)
			pszMotion = const_cast<char*>(pszDefault);
	}

	SService<IPuppetSrv> pMotSrv(g_pScriptManager);
	pMotSrv->PlayMotion(__p, ObjId(), pszMotion);

	if (pszMotion != pszDefault)
		g_pMalloc->Free(pszMotion);
}

void cScr_FactoryWork::FrobLever(object iTarget)
{
	PlayMotion("frobmotion", g_pszDefaultFrobMotion);

	SService<IAIScrSrv> pAISrv(g_pScriptManager);
	true_bool __p;
	pAISrv->MakeFrobObj(__p, ObjId(), iTarget, kNormalPriorityAction, cMultiParm::Undef);
	//pAISrv->SetScriptFlags(ObjId(), 2);
}

void cScr_FactoryWork::ChangeHammerModel(bool bWithHammer)
{
	sLink slMP;
	if (GetOneLinkByDataInheritedSrc("ScriptParams", ObjId(), 0, &slMP, "WithHammer", -1))
	{
		if (bWithHammer)
			AddSingleMetaProperty(slMP.dest, ObjId());
		else
			RemoveSingleMetaProperty(slMP.dest, ObjId());
	}
	else
	{
		if (bWithHammer)
			AddSingleMetaProperty("M-WithHammer", ObjId());
		else
			RemoveSingleMetaProperty("M-WithHammer", ObjId());
	}
}

void cScr_FactoryWork::PickUp(object iHammer)
{
	SService<IAIScrSrv> pAISrv(g_pScriptManager);
	true_bool __p;
	pAISrv->MakeFrobObj(__p, ObjId(), iHammer, kNormalPriorityAction, cMultiParm::Undef);
}

void cScr_FactoryWork::MakeHammer(void)
{
	SService<IObjectSrv> pOS(g_pScriptManager);
	object oArc;
	object oHammer;
	pOS->Named(oArc, "Molds");
	object iMold = GetOwned(oArc);
	sLink slProduct;
	if (GetOneLinkInheritedSrc("Contains", iMold, 0, &slProduct))
	{
		pOS->Create(oHammer, slProduct.dest);
	}
	else
	{
		pOS->Named(oArc, "Warhammer");
		pOS->Create(oHammer, oArc);
	}

	SService<IContainSrv> pCS(g_pScriptManager);
	pCS->Add(oHammer, ObjId(), 0, 1);

	ChangeHammerModel(true);
}

void cScr_FactoryWork::DecideAction(void)
{
	object iHammer, iMold, iMoldLvr;

	if (m_iCurrentAction == kWorkKeepHammer)
	{
		DebugString("Not acting.");
		return;
	}

	iHammer = GetOwned(StrToObject("Hammers"));
	iMold = GetOwned(StrToObject("Molds"));
	iMoldLvr = WhatControls(iMold);
	if (iHammer)
	{
		if (IsContained(iHammer))
		{
			DebugString("Action: DropHammer");
			m_iCurrentAction = kWorkDropHammer;
			GotoWayPt(GetPath());
		}
		else
		{
			DebugString("Action: PickUpHammer");
			m_iCurrentAction = kWorkPickUpHammer;
			GotoWayPt(GetOwned(StrToObject("Molds")));
		}
	}
	else if (iMold && IsMoldFull(iMold))
	{
		if (! IsLocked(iMoldLvr))
		{
			DebugString("Action: MoldLeverOn");
			m_iCurrentAction = kWorkMoldLeverOn;
			GotoWayPt(iMoldLvr);
		}
		else
		{
			DebugString("Action: CauldLeverOff");
			m_iCurrentAction = kWorkCauldLeverOff;
			GotoWayPt(WhatControls(GetOwned(StrToObject("SmeltingCauldr"))));
		}
	}
	else if (iMoldLvr && ! IsLeverOn(iMoldLvr))
	{
		DebugString("Action: CauldLeverOn");
		object iCauldLvr = WhatControls(GetOwned(StrToObject("SmeltingCauldr")));
		m_iCurrentAction = kWorkCauldLeverOn;
		if (! IsLeverOn(iCauldLvr))
		{
			GotoWayPt(iCauldLvr);
		}
	}
	else
	{
		DebugString("Action: MoldLeverOff");
		m_iCurrentAction = kWorkMoldLeverOff;
		GotoWayPt(iMoldLvr);
	}
}

long cScr_FactoryWork::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	m_iCurrentAction.Init(0);
	m_iWayPtDest.Init(0);

	return cBaseAIScript::OnBeginScript(pMsg, mpReply);
}

long cScr_FactoryWork::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
	if (pSimMsg->fStarting)
	{
		m_iCurrentAction = 0;
		m_iWayPtDest = 0;

		DebugString("Starting Work");
		DecideAction();
	}

	return cBaseAIScript::OnSim(pSimMsg, mpReply);
}

long cScr_FactoryWork::OnSlain(sSlayMsg* pSlayMsg, cMultiParm& mpReply)
{
	RemoveSingleMetaProperty("M-FactoryWorker", ObjId());

	return cBaseAIScript::OnSlain(pSlayMsg, mpReply);
}

long cScr_FactoryWork::OnAIModeChange(sAIModeChangeMsg* pAIModeMsg, cMultiParm& mpReply)
{
	if (pAIModeMsg->mode == kAIM_Dead)
	{
		RemoveSingleMetaProperty("M-FactoryWorker", ObjId());
	}

	return cBaseAIScript::OnAIModeChange(pAIModeMsg, mpReply);
}

long cScr_FactoryWork::OnAlertness(sAIAlertnessMsg* pAlertMsg, cMultiParm& mpReply)
{
	if (pAlertMsg->level == kHighAlert
	 && m_iCurrentAction != kWorkKeepHammer)
	{
		/*
		SService<IAIScrSrv> pAISrv(g_pScriptManager);
		pAISrv->ClearGoals(ObjId());
		pAISrv->SetScriptFlags(ObjId(), 4);
		*/

		DebugString("Aborting Work");
		m_iCurrentAction = kWorkKeepHammer;
		object iHammer = GetOwned(StrToObject("Hammers"));
		if (iHammer)
		{
			if (IsContained(iHammer))
				RemoveSingleMetaProperty("M-FactoryWorker", ObjId());
			else
				GotoWayPt(GetOwned(StrToObject("Molds")), kHighPriorityAction);
		}
		else
		{
			GotoWayPt(GetPath(), kHighPriorityAction);
		}
	}

	return cBaseAIScript::OnAlertness(pAlertMsg, mpReply);
}

long cScr_FactoryWork::OnObjActResult(sAIObjActResultMsg* pActionMsg, cMultiParm& mpReply)
{
	if (pActionMsg->result == kActionDone)
	{
		if (pActionMsg->action == kAIGoto)
		{
			if (m_iWayPtDest != pActionMsg->target)
			{
				GotoWayPt(int(m_iWayPtDest), (m_iCurrentAction == kWorkKeepHammer) ?
						  kHighPriorityAction : kNormalPriorityAction);
			}
			else
			{
				switch (m_iCurrentAction)
				{
				case kWorkCauldLeverOn:
				case kWorkMoldLeverOn:
					if (! IsLeverOn(pActionMsg->target))
						FrobLever(pActionMsg->target);
					else
						DecideAction();
					break;
				case kWorkMoldLeverOff:
				case kWorkCauldLeverOff:
					if (IsLeverOn(pActionMsg->target))
						FrobLever(pActionMsg->target);
					else
						DecideAction();
					break;
				case kWorkPickUpHammer:
				case kWorkKeepHammer:
					PlayMotion("pickupmotion", g_pszDefaultPickupMotion);
					break;
				case kWorkDropHammer:
					PlayMotion("dropmotion", g_pszDefaultDropMotion);
					break;
				default:
					DecideAction();
					break;
				}
			}
		}
		else if (pActionMsg->action == kAIFrob)
		{
			switch (m_iCurrentAction)
			{
			case kWorkCauldLeverOn:
				break;
			case kWorkMoldLeverOff:
			case kWorkCauldLeverOff:
			case kWorkMoldLeverOn:
			case kWorkDropHammer:
				DecideAction();
				break;
			case kWorkPickUpHammer:
				if (IsContained(pActionMsg->target))
					ChangeHammerModel(true);
				DecideAction();
				break;
			case kWorkKeepHammer:
				if (IsContained(pActionMsg->target))
					ChangeHammerModel(true);
				else
					MakeHammer();
				RemoveSingleMetaProperty("M-FactoryWorker", ObjId());
				break;
			default:
				DecideAction();
				break;
			}
		}
	}

	return cBaseAIScript::OnObjActResult(pActionMsg, mpReply);
}

long cScr_FactoryWork::OnMotionEnd(sBodyMsg* pMotionMsg, cMultiParm& mpReply)
{
	//SService<IAIScrSrv> pAISrv(g_pScriptManager);
	//pAISrv->SetScriptFlags(ObjId(), 0);

	switch (m_iCurrentAction)
	{
	case kWorkPickUpHammer:
	{
		object iHammer = GetOwned(StrToObject("Hammers"));
		if (iHammer)
		{
			PickUp(iHammer);
		}
		else
		{
			DebugString("Hey! Where's my hammer!");
			DecideAction();
		}
		break;
	}
	case kWorkDropHammer:
	{
		object iHammer = GetOwned(StrToObject("Hammers"));
		if (iHammer && IsContained(iHammer))
		{
			SService<IObjectSrv> pOS(g_pScriptManager);
			pOS->Destroy(iHammer);
		}
		ChangeHammerModel(false);
		m_iCurrentAction = kWorkDecideAction;
		GotoWayPt(GetOwned(StrToObject("TrolPt")));
		break;
	}
	case kWorkKeepHammer:
	{
		object iHammer = GetOwned(StrToObject("Hammers"));
		if (iHammer)
			PickUp(iHammer);
		else
		{
			MakeHammer();
			RemoveSingleMetaProperty("M-FactoryWorker", ObjId());
		}
		return 0;
	}
	default:
		DecideAction();
		break;
	}

	return cBaseAIScript::OnMotionEnd(pMotionMsg, mpReply);
}

long cScr_FactoryWork::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "Report"))
	{
		if (!strcmp(static_cast<const char*>(pMsg->data), "Mold")
		 && m_iCurrentAction == kWorkCauldLeverOn)
			DecideAction();
		return 0;
	}

	return cBaseAIScript::OnMessage(pMsg, mpReply);
}


/***
 * Prisoner
 */
long cScr_Prisoner::OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IObjectSrv> pOS(g_pScriptManager);
	object oMP;
	pOS->Named(oMP, "M-Escapee");
	if (! oMP)
		return 1;

	true_bool bEscapee;
	pOS->InheritsFrom(bEscapee, ObjId(), oMP);
	if (! bEscapee)
	{
		SService<IAIScrSrv> pAISrv(g_pScriptManager);
		pAISrv->Signal(ObjId(), "Escape");
	}

	return cBaseScript::OnTurnOn(pMsg, mpReply);
}


/***
 * ResetLockbox
 */
void cScr_LockResetter::StartTimer(void)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "CfgTweqBlink"))
	{
		if (! pPS->Possessed(ObjId(), "StTweqBlink"))
			pPS->Add(ObjId(), "StTweqBlink");
		pPS->Set(ObjId(), "StTweqBlink", "Cur Time", 0);
		pPS->Set(ObjId(), "StTweqBlink", "Frame #", 1);

		SService<IActReactSrv> pARSrv(g_pScriptManager);
		pARSrv->React(pARSrv->GetReactionNamed("tweq_control"), 1.0f, ObjId(), 0,
			kTweqTypeFlicker, kTweqDoActivate,
			cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef);
	}
}

long cScr_LockResetter::OnNowUnlocked(sScrMsg* pMsg, cMultiParm& mpReply)
{
	StartTimer();

	return cBaseScript::OnNowUnlocked(pMsg, mpReply);
}

long cScr_LockResetter::OnTweqComplete(sTweqMsg* pTweqMsg, cMultiParm& mpReply)
{
	if (pTweqMsg->Type == kTweqTypeFlicker)
	{
		SService<ILinkSrv> pLS(g_pScriptManager);
		SService<ILinkToolsSrv> pLTS(g_pScriptManager);
		linkset lsLocks;
		pLS->GetAll(lsLocks, pLTS->LinkKindNamed("~Lock"), ObjId(), 0);
		for (; lsLocks.AnyLinksLeft(); lsLocks.NextLink())
		{
			sLink sl = lsLocks.Get();
			true_bool bDoorInUse;
			pLS->AnyExist(bDoorInUse, pLTS->LinkKindNamed("~AIDoor"), sl.dest, 0);
			if (bDoorInUse)
			{
				StartTimer();
				return 0;
			}
		}

		SService<IPropertySrv> pPS(g_pScriptManager);
		pPS->SetSimple(ObjId(), "Locked", 1);
	}

	return cBaseScript::OnTweqComplete(pTweqMsg, mpReply);
}


/***
 * TrapSecureDoor
 */
cAnsiStr cScr_SuspiciousTrap::GetWatchers(void)
{
	cAnsiStr strWatchers;
	char* pszParam = GetObjectParamString(ObjId(), "watcher");
	if (pszParam)
	{
		strWatchers = "@";
		strWatchers += pszParam;
		g_pMalloc->Free(pszParam);
	}
	else
		strWatchers = "@Human";
	return strWatchers;
}

long cScr_SuspiciousTrap::OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "SuspObj"))
	{
		pPS->Set(ObjId(), "SuspObj", "Is Suspicious", 1);
	}
	else
	{
		SService<ILinkSrv> pLS(g_pScriptManager);
		SService<ILinkToolsSrv> pLTS(g_pScriptManager);
		cAnsiStr strWatchers = GetWatchers();
		char szSelf[12];
		sprintf(szSelf, "%d", ObjId());
		pLS->DestroyMany(pLTS->LinkKindNamed("AIWatchObj"), static_cast<const char*>(strWatchers), szSelf);
		pLS->CreateMany(pLTS->LinkKindNamed("AIWatchObj"), static_cast<const char*>(strWatchers), szSelf);
	}

	return cBaseScript::OnTurnOn(pMsg, mpReply);
}

long cScr_SuspiciousTrap::OnTurnOff(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "SuspObj"))
	{
		pPS->Set(ObjId(), "SuspObj", "Is Suspicious", 0);
	}
	else
	{
		SService<ILinkSrv> pLS(g_pScriptManager);
		SService<ILinkToolsSrv> pLTS(g_pScriptManager);
		cAnsiStr strWatchers = GetWatchers();
		char szSelf[12];
		sprintf(szSelf, "%d", ObjId());
		pLS->DestroyMany(pLTS->LinkKindNamed("AIWatchObj"), static_cast<const char*>(strWatchers), szSelf);
	}

	return cBaseScript::OnTurnOff(pMsg, mpReply);
}


/***
 * TrigOBBPlayerStuff
 */
long cScr_OBBPlayerStuff::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->SubscribeMsg(ObjId(), kPhysEnterExit);

	return cBaseTrap::OnBeginScript(pMsg, mpReply);
}

long cScr_OBBPlayerStuff::OnEndScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->UnsubscribeMsg(ObjId(), kPhysEnterExit);

	return cBaseTrap::OnEndScript(pMsg, mpReply);
}

long cScr_OBBPlayerStuff::OnPhysEnter(sPhysMsg* pPhysMsg, cMultiParm& mpReply)
{
	int iPlayer = StrToObject("Player");
	if (pPhysMsg->transObj == iPlayer)
	{
		DoTrigger(true);
	}
	else
	{
		SService<ILinkSrv> pLS(g_pScriptManager);
		SService<ILinkToolsSrv> pLTS(g_pScriptManager);
		true_bool bFirer;
		if (*pLS->AnyExist(bFirer, pLTS->LinkKindNamed("Firer"), pPhysMsg->transObj, iPlayer))
		{
			DoTrigger(true);
		}
	}

	return cBaseTrap::OnPhysEnter(pPhysMsg, mpReply);
}

long cScr_OBBPlayerStuff::OnPhysExit(sPhysMsg* pPhysMsg, cMultiParm& mpReply)
{
	return cBaseTrap::OnPhysExit(pPhysMsg, mpReply);
}


/***
 * CuttyCell
 */
long cScr_ConvRoomPlayer::OnPlayerRoomEnter(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	CDSend("Presence", ObjId());

	return cBaseRoomScript::OnPlayerRoomEnter(pRoomMsg, mpReply);
}

long cScr_ConvRoomPlayer::OnPlayerRoomExit(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	CDSend("Absence", ObjId());

	return cBaseRoomScript::OnPlayerRoomExit(pRoomMsg, mpReply);
}


/***
 * NearCuttyCell
 */
/*
void cScr_ConvRoomOpponent::TrackCreatureEnter(int iObj)
{
	AddMetaProperty("M-NotifyRegion", iObj);
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	link lPopLink;
	pLS->Create(lPopLink, pLTS->LinkKindNamed("Population"), ObjId(), iObj);
}

void cScr_ConvRoomOpponent::TrackCreatureExit(int iObj)
{
	RemoveMetaProperty("M-NotifyRegion", iObj);
	DeleteAllLinks("Population", ObjId(), iObj);
}
*/

bool cScr_ConvRoomOpponent::IsOpponent(object iObj)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	cMultiParm mpTeam;
	pPS->Get(mpTeam, iObj, "AI_Team", NULL);
	return static_cast<int>(mpTeam) > 1;
}

long cScr_ConvRoomOpponent::OnCreatureRoomEnter(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	if (IsOpponent(pRoomMsg->MoveObjId))
	{
		if (0 == Population())
		{
			CDSend("Intrusion", ObjId());
		}
		TrackCreatureEnter(pRoomMsg->MoveObjId);
	}

	return cBaseRoomScript::OnCreatureRoomEnter(pRoomMsg, mpReply);
}

long cScr_ConvRoomOpponent::OnCreatureRoomExit(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	if (IsOpponent(pRoomMsg->MoveObjId))
	{
		TrackCreatureExit(pRoomMsg->MoveObjId);
		if (0 == Population())
		{
			CDSend("Privacy", ObjId());
		}
	}

	return cBaseRoomScript::OnCreatureRoomExit(pRoomMsg, mpReply);
}

long cScr_ConvRoomOpponent::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (pMsg->from != 0 && !_stricmp(pMsg->message, "Obituary"))
	{
		TrackCreatureExit(pMsg->from);
		if (0 == Population())
		{
			CDSend("Privacy", ObjId());
		}
	}

	return cBaseRoomScript::OnMessage(pMsg, mpReply);
}


/***
 * ConvControl
 */
cScr_ConvControl::cScr_ConvControl(const char* pszName, int iHostObjId)
	: cBaseScript(pszName, iHostObjId),
	  SCRIPT_VAROBJ(cScr_ConvControl,m_active,iHostObjId),
	  SCRIPT_VAROBJ(cScr_ConvControl,m_busy,iHostObjId),
	  SCRIPT_VAROBJ(cScr_ConvControl,m_intruded,iHostObjId)
{
	static const sMessageHandler handlers[] = {
		{"Presence", HandlePresence},
		{"Absence", HandleAbsence},
		{"Intrusion", HandleIntrusion},
		{"Privacy", HandlePrivacy},
		{"ConvNext", HandleConvNext},
		{"ConvEnd", HandleConvEnd}};
	RegisterMessageHandlers(handlers, 6);
}

long cScr_ConvControl::HandlePresence(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cScr_ConvControl*>(pScript)->OnTurnOn(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cScr_ConvControl::HandleAbsence(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cScr_ConvControl*>(pScript)->OnTurnOff(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cScr_ConvControl::HandleIntrusion(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cScr_ConvControl*>(pScript)->OnNowLocked(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cScr_ConvControl::HandlePrivacy(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cScr_ConvControl*>(pScript)->OnNowUnlocked(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cScr_ConvControl::HandleConvNext(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cScr_ConvControl*>(pScript)->OnConvNext(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cScr_ConvControl::HandleConvEnd(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cScr_ConvControl*>(pScript)->OnConvEnd(pMsg, static_cast<cMultiParm&>(*pReply));
}

void cScr_ConvControl::KillPrevLink(void)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	link lCD;
	pLS->GetOne(lCD, pLTS->LinkKindNamed("ControlDevice"), ObjId(), 0);
	if (lCD)
	{
		DebugPrintf("ConvControl: Removing link #%08lX\n", static_cast<long>(lCD));
		pLS->Destroy(lCD);
	}
}

void cScr_ConvControl::RunNextLink(void)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	link lCD;
	pLS->GetOne(lCD, pLTS->LinkKindNamed("ControlDevice"), ObjId(), 0);
	if (lCD)
	{
		sLink sl;
		pLTS->LinkGet(lCD, sl);
		DebugPrintf("ConvControl: Triggering %d\n", static_cast<int>(sl.dest));
		PostMessage(sl.dest, "TurnOn");
		m_busy = 1;
	}
	else
	{
		DebugPrintf("ConvControl: Nothing to do!\n");
		m_busy = 0;
		m_active = 0;
	}
}

long cScr_ConvControl::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	m_active.Init(0);
	m_busy.Init(0);
	m_intruded.Init(0);

	return cBaseScript::OnBeginScript(pMsg, mpReply);
}

long cScr_ConvControl::OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (! m_active)
	{
		m_active = 1;
		if (! m_busy && ! m_intruded)
			RunNextLink();
	}

	return cBaseScript::OnTurnOn(pMsg, mpReply);
}

long cScr_ConvControl::OnTurnOff(sScrMsg* pMsg, cMultiParm& mpReply)
{
	m_active = 0;

	return cBaseScript::OnTurnOff(pMsg, mpReply);
}

long cScr_ConvControl::OnNowLocked(sScrMsg* pMsg, cMultiParm& mpReply)
{
	m_intruded = 1;

	return cBaseScript::OnNowLocked(pMsg, mpReply);
}

long cScr_ConvControl::OnNowUnlocked(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (m_intruded)
	{
		m_intruded = 0;
		if (m_active && ! m_busy)
			RunNextLink();
	}

	return cBaseScript::OnNowUnlocked(pMsg, mpReply);
}

long cScr_ConvControl::OnConvNext(sScrMsg*, cMultiParm& mpReply)
{
	if (m_busy)
		KillPrevLink();
	if (m_active && ! m_intruded)
		RunNextLink();
	else
		m_busy = 0;

	mpReply = 1;
	return 0;
}

long cScr_ConvControl::OnConvEnd(sScrMsg*, cMultiParm& mpReply)
{
	if (m_busy)
	{
		KillPrevLink();
		m_busy = 0;
	}
	m_active = 0;

	mpReply = 1;
	return 0;
}


/***
 * ConvSpeaker
 */
object cScr_ConvSpeaker::FindController(void)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	linkkind lkCD = pLTS->LinkKindNamed("~ControlDevice");
	link lCD;
	linkset lsConv;
	pLS->GetAll(lsConv, pLTS->LinkKindNamed("~AIConversationActor"), ObjId(), 0);
	for (; lsConv.AnyLinksLeft(); lsConv.NextLink())
	{
		sLink sl = lsConv.Get();
		pLS->GetOne(lCD, lkCD, sl.dest, 0);
		if (lCD)
		{
			pLTS->LinkGet(lCD, sl);
			return sl.dest;
		}
	}
	DebugString("No conversation controller!");
	return 0;
}

int cScr_ConvSpeaker::GetObjList(const char* arg, int objs[24])
{
	if (!arg)
		return 0;
	int nGoals = 0;
	char* p = const_cast<char*>(arg);
	char* q = NULL;
	int i;
	while (nGoals < 24)
	{
		while (isspace(*p)) ++p;
		i = strtol(p, &q, 10);
		if (q == p)
			break;
		objs[nGoals++] = i;
		while (isspace(*q) || *q == ',') ++q;
		p = q;
	}
	return nGoals;
}

bool cScr_ConvSpeaker::DoObjComplete(const char* arg)
{
	SService<IQuestSrv> pQS(g_pScriptManager);
	char qvar[24];
	int objs[24];
	int nGoals = GetObjList(arg, objs);
	for (int n = 0; n < nGoals; ++n)
	{
		sprintf(qvar, "goal_state_%d", objs[n]);
		pQS->Set(qvar, 1, kQuestDataMission);
	}
	return true;
}

bool cScr_ConvSpeaker::DoObjFail(const char* arg)
{
	SService<IQuestSrv> pQS(g_pScriptManager);
	char qvar[24];
	int objs[24];
	int nGoals = GetObjList(arg, objs);
	for (int n = 0; n < nGoals; ++n)
	{
		sprintf(qvar, "goal_state_%d", objs[n]);
		pQS->Set(qvar, 3, kQuestDataMission);
	}
	return true;
}

bool cScr_ConvSpeaker::DoObjInvalid(const char* arg)
{
	SService<IQuestSrv> pQS(g_pScriptManager);
	char qvar[24];
	int objs[24];
	int nGoals = GetObjList(arg, objs);
	for (int n = 0; n < nGoals; ++n)
	{
		sprintf(qvar, "goal_state_%d", objs[n]);
		pQS->Set(qvar, 2, kQuestDataMission);
	}
	return true;
}

bool cScr_ConvSpeaker::DoObjShow(const char* arg)
{
	SService<IQuestSrv> pQS(g_pScriptManager);
	char qvar[24];
	int objs[24];
	int nGoals = GetObjList(arg, objs);
	for (int n = 0; n < nGoals; ++n)
	{
		sprintf(qvar, "goal_visible_%d", objs[n]);
		pQS->Set(qvar, 1, kQuestDataMission);
	}
	return true;
}

bool cScr_ConvSpeaker::DoObjHide(const char* arg)
{
	SService<IQuestSrv> pQS(g_pScriptManager);
	char qvar[24];
	int objs[24];
	int nGoals = GetObjList(arg, objs);
	for (int n = 0; n < nGoals; ++n)
	{
		sprintf(qvar, "goal_visible_%d", objs[n]);
		pQS->Set(qvar, 0, kQuestDataMission);
	}
	return true;
}

bool cScr_ConvSpeaker::DoObjReplace(const char* arg1, const char* arg2)
{
	SService<IQuestSrv> pQS(g_pScriptManager);
	char qvar[24];
	int objs[24];
	int nGoals = GetObjList(arg2, objs);
	for (int n = 0; n < nGoals; ++n)
	{
		sprintf(qvar, "goal_visible_%d", objs[n]);
		pQS->Set(qvar, 1, kQuestDataMission);
	}
	nGoals = GetObjList(arg1, objs);
	for (int n = 0; n < nGoals; ++n)
	{
		sprintf(qvar, "goal_visible_%d", objs[n]);
		pQS->Set(qvar, 0, kQuestDataMission);
	}
	return true;
}

bool cScr_ConvSpeaker::DoObjIsShown(const char* arg)
{
	SService<IQuestSrv> pQS(g_pScriptManager);
	char qvar[24];
	int objs[24];
	int nGoals = GetObjList(arg, objs);
	for (int n = 0; n < nGoals; ++n)
	{
		sprintf(qvar, "goal_visible_%d", objs[n]);
		if (pQS->Get(qvar) == 0)
			return false;
	}
	return true;
}

bool cScr_ConvSpeaker::DoObjIsComplete(const char* arg)
{
	SService<IQuestSrv> pQS(g_pScriptManager);
	char qvar[24];
	int objs[24];
	int nGoals = GetObjList(arg, objs);
	for (int n = 0; n < nGoals; ++n)
	{
		sprintf(qvar, "goal_state_%d", objs[n]);
		if (pQS->Get(qvar) != 1)
			return false;
	}
	return true;
}

bool cScr_ConvSpeaker::DoVoiceOver(const char* arg)
{
	int iSchema = StrToObject(arg);
	if (iSchema)
	{
		SService<ISoundScrSrv> pSounds(g_pScriptManager);
		true_bool bSuccess;
		pSounds->PlayVoiceOver(bSuccess, ObjId(), iSchema);
		return bSuccess;
	}
	return false;
}

bool cScr_ConvSpeaker::DoAmbient(const char* arg)
{
	int iSchema = StrToObject(arg);
	if (iSchema)
	{
		SService<ISoundScrSrv> pSounds(g_pScriptManager);
		true_bool bSuccess;
		pSounds->PlaySchemaAmbient(bSuccess, ObjId(), iSchema, kSoundNetNormal);
		return bSuccess;
	}
	return false;
}

bool cScr_ConvSpeaker::DoSlay(const char* arg)
{
	int iVictim;
	if (!arg || !arg[0] || !_stricmp(arg, "self"))
		iVictim = ObjId();
	else
		iVictim = StrToObject(arg);
	if (iVictim)
	{
		SService<IDamageSrv> pDmg(g_pScriptManager);
		pDmg->Slay(iVictim, ObjId());
	}
	return true;
}

bool cScr_ConvSpeaker::DoConvNext(const char* arg)
{
	int iController = StrToObject(arg);
	if (!iController)
		iController = FindController();
	if (iController)
		PostMessage(iController, "ConvNext");
	return true;
}

bool cScr_ConvSpeaker::DoConvEnd(const char* arg)
{
	int iController = StrToObject(arg);
	if (!iController)
		iController = FindController();
	if (iController)
		PostMessage(iController, "ConvEnd");
	return true;
}

long cScr_ConvSpeaker::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	int bSuccess = true;
	if (!_stricmp(pMsg->message, "ConvNext"))
		bSuccess = DoConvNext(pMsg->data);
	else if (!_stricmp(pMsg->message, "ConvEnd"))
		bSuccess = DoConvEnd(pMsg->data);
	else if (!_stricmp(pMsg->message, "Slay"))
		bSuccess = DoSlay(pMsg->data);
	else if (!_stricmp(pMsg->message, "PlayVO"))
		bSuccess = DoVoiceOver(pMsg->data);
	else if (!_stricmp(pMsg->message, "PlayAmbient"))
		bSuccess = DoAmbient(pMsg->data);
	else if (!_stricmp(pMsg->message, "CompleteGoal"))
		bSuccess = DoObjComplete(pMsg->data);
	else if (!_stricmp(pMsg->message, "FailGoal"))
		bSuccess = DoObjFail(pMsg->data);
	else if (!_stricmp(pMsg->message, "CancelGoal"))
		bSuccess = DoObjInvalid(pMsg->data);
	else if (!_stricmp(pMsg->message, "ShowGoal"))
		bSuccess = DoObjShow(pMsg->data);
	else if (!_stricmp(pMsg->message, "HideGoal"))
		bSuccess = DoObjHide(pMsg->data);
	else if (!_stricmp(pMsg->message, "SwapGoal"))
		bSuccess = DoObjReplace(pMsg->data, pMsg->data2);
	else if (!_stricmp(pMsg->message, "IsGoalShown"))
		bSuccess = DoObjIsShown(pMsg->data);
	else if (!_stricmp(pMsg->message, "IsGoalComplete"))
		bSuccess = DoObjIsComplete(pMsg->data);

	mpReply = bSuccess;

	return cBaseScript::OnMessage(pMsg, mpReply);
}


/***
 * HeStartedIt
 */
void cScr_PickAFight::CheckCulpability(object iSource)
{
	object iPlayer = StrToObject("Player");
	if (iPlayer == iSource
	 || iPlayer == GetOneLinkDest("~CulpableFor", iSource))
	{
		SService<IObjectSrv> pOS(g_pScriptManager);
		object oMP;
		pOS->Named(oMP, "M-Swaying Burrick");
		if (oMP)
			pOS->RemoveMetaPropertyFromMany(oMP, "@Creature");
	}
}

long cScr_PickAFight::OnSlain(sSlayMsg* pSlayMsg, cMultiParm& mpReply)
{
	CheckCulpability(pSlayMsg->culprit);

	return cBaseScript::OnSlain(pSlayMsg, mpReply);
}

long cScr_PickAFight::OnDamage(sDamageScrMsg* pDmgMsg, cMultiParm& mpReply)
{
	CheckCulpability(pDmgMsg->culprit);

	return cBaseScript::OnDamage(pDmgMsg, mpReply);
}

long cScr_PickAFight::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "KnockoutStimulus"))
	{
		SService<ILinkToolsSrv> pLTS(g_pScriptManager);
		sLink sl;
		pLTS->LinkGet(static_cast<sStimMsg*>(pMsg)->source, sl);
		CheckCulpability(sl.source);
	}

	return cBaseScript::OnMessage(pMsg, mpReply);
}


/***
 * Horn
 */
long cScr_AccornOfQuintuplets::OnContained(sContainedScrMsg* pContMsg, cMultiParm& mpReply)
{
	SService<IObjectSrv> pOS(g_pScriptManager);
	object oMP;
	pOS->Named(oMP, "M-Swaying Burrick");
	if (oMP)
		pOS->RemoveMetaPropertyFromMany(oMP, "@Creature");

	SService<IPropertySrv> pPS(g_pScriptManager);
	pPS->Remove(ObjId(), "AmbientHacked");

	return cBaseScript::OnContained(pContMsg, mpReply);
}


/***
 * MagicBones
 */
long cScr_MagicBone::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->SubscribeMsg(ObjId(), kPhysCollision);

	return cBaseScript::OnBeginScript(pMsg, mpReply);
}

long cScr_MagicBone::OnEndScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->UnsubscribeMsg(ObjId(), kPhysCollision);

	return cBaseScript::OnEndScript(pMsg, mpReply);
}

long cScr_MagicBone::OnPhysCollision(sPhysMsg* pPhysMsg, cMultiParm& mpReply)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	true_bool bOwns;
	pLS->AnyExist(bOwns, pLTS->LinkKindNamed("Owns"), pPhysMsg->collObj, ObjId());
	if (bOwns)
		mpReply = sPhysMsg::kSlay;

	return cBaseScript::OnPhysCollision(pPhysMsg, mpReply);
}

long cScr_MagicBone::OnSlain(sSlayMsg* pSlayMsg, cMultiParm& mpReply)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	pLS->BroadcastOnAllLinks(ObjId(), "BoneSlain", pLTS->LinkKindNamed("~Owns"));

	return cBaseScript::OnSlain(pSlayMsg, mpReply);
}


/***
 * MagicCoffin
 */
long cScr_MagicCoffin::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "BoneSlain"))
	{
		SService<ILinkSrv> pLS(g_pScriptManager);
		SService<ILinkToolsSrv> pLTS(g_pScriptManager);
		true_bool bOwns;
		pLS->AnyExist(bOwns, pLTS->LinkKindNamed("Owns"), ObjId(), 0);
		if (! bOwns)
			DoTrigger(true);

		return 0;
	}

	return cBaseTrap::OnMessage(pMsg, mpReply);
}


/***
 * TrapAIWake
 */
long cScr_WakeTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
	{
		CDSend("WakeyWakey", ObjId());
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * WakeableAI
 */
long cScr_WakeAI::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "WakeyWakey"))
	{
		SService<IPropertySrv> pPS(g_pScriptManager);
		if (pPS->Possessed(ObjId(), "AI_Mode"))
		{
			cMultiParm mpMode;
			pPS->Get(mpMode, ObjId(), "AI_Mode", NULL);
			if (static_cast<int>(mpMode) == kAIM_Asleep)
			{
				mpMode = kAIM_Normal;
				pPS->SetSimple(ObjId(), "AI_Mode", mpMode);
			}
		}

		return 0;
	}

	return cBaseScript::OnMessage(pMsg, mpReply);
}


/***
 * CleanObjDestroy
 */
bool cScr_PoliteDestroyTrap::AttemptDestroy(void)
{
	bool bSuccess = true;

	SService<IObjectSrv> pOS(g_pScriptManager);
	SService<IPropertySrv> pPS(g_pScriptManager);
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	SService<ISoundScrSrv> pSS(g_pScriptManager);

	object oPlayer;
	pOS->Named(oPlayer, "Player");

	linkkind lCont = pLTS->LinkKindNamed("Contains");

	linkset lsLinks;
	pLS->GetAll(lsLinks, pLTS->LinkKindNamed("ControlDevice"), ObjId(), 0);
	for (; lsLinks.AnyLinksLeft(); lsLinks.NextLink())
	{
		sLink sl = lsLinks.Get();

		true_bool bDontDestroy;
		pOS->RenderedThisFrame(bDontDestroy, sl.dest);
		if (bDontDestroy)
		{
			bSuccess = false;
			continue;
		}
		pLS->AnyExist(bDontDestroy, lCont, oPlayer, sl.dest);
		if (bDontDestroy)
		{
			bSuccess = false;
			continue;
		}

		linkset lsConts;
		pLS->GetAll(lsConts, lCont, sl.dest, 0);
		for (; lsConts.AnyLinksLeft(); lsConts.NextLink())
		{
			sLink cont = lsConts.Get();
			pOS->Destroy(cont.dest);
		}

		if (pPS->Possessed(sl.dest, "RotDoor") || pPS->Possessed(sl.dest, "TransDoor"))
		{
			SService<IDoorSrv> pDS(g_pScriptManager);
			pDS->OpenDoor(sl.dest);
		}

		pSS->HaltSpeech(sl.dest);
		pOS->Destroy(sl.dest);
	}

	return bSuccess;
}

long cScr_PoliteDestroyTrap::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (! strcmp(pTimerMsg->name, "PoliteDestroy"))
	{
		if (! AttemptDestroy())
			SetTimedMessage("PoliteDestroy", 30000, kSTM_OneShot);

		return 0;
	}

	return cBaseTrap::OnTimer(pTimerMsg, mpReply);
}

long cScr_PoliteDestroyTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
	{
		if (! AttemptDestroy())
			SetTimedMessage("PoliteDestroy", 20000, kSTM_OneShot);
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


#endif // _DARKGAME == 2
