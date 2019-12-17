#include "cbase.h"
#include "c_basetempentity.h"
#include "engine/IEngineSound.h"
#include "weapon/weapon_base.h"

#include "tier0/memdbgon.h"

class C_TETFExplosion : public C_BaseTempEntity
{
  public:
    DECLARE_CLASS(C_TETFExplosion, C_BaseTempEntity);
    DECLARE_CLIENTCLASS();

    C_TETFExplosion();

protected:
    void PostDataUpdate(DataUpdateType_t updateType) OVERRIDE;

private:
    Vector m_vecOrigin;
    Vector m_vecNormal;
    CWeaponID m_iWeaponID;
};

C_TETFExplosion::C_TETFExplosion()
{
    m_vecOrigin.Init();
    m_vecNormal.Init();
    m_iWeaponID = WEAPON_NONE;
}

void C_TETFExplosion::PostDataUpdate(DataUpdateType_t updateType)
{
    if (updateType == DATA_UPDATE_CREATED)
    {
        const auto pWeaponInfo = GetWeaponInfo(m_iWeaponID);
        AssertMsg(pWeaponInfo, "Invalid pWeaponInfo for weaponID %i\n", m_iWeaponID);
        if (!pWeaponInfo)
            return;

        const bool bIsWater = (UTIL_PointContents(m_vecOrigin) & CONTENTS_WATER);
        // Cannot use zeros here because we are sending the normal at a smaller bit size.
        const bool bInAir = fabs(m_vecNormal.x) < 0.05f && fabs(m_vecNormal.y) < 0.05f && fabs(m_vecNormal.z) < 0.05f;
        QAngle angExplosion(0.0f, 0.0f, 0.0f);

        if (!bInAir)
            VectorAngles(m_vecNormal, angExplosion);

        static ConVarRef mom_rj_particles("mom_rj_particles");
        static ConVarRef mom_rj_sounds("mom_rj_sounds");

        if (mom_rj_sounds.GetInt() > 0)
        {
            const char *pszSound = pWeaponInfo->aShootSounds[EXPLOSION];
            if (mom_rj_sounds.GetInt() == 2)
                pszSound = "BaseExplosionEffect.SoundTF2";

            CLocalPlayerFilter filter;
            C_BaseEntity::EmitSound(filter, SOUND_FROM_WORLD, pszSound, &m_vecOrigin);
        }

        if (mom_rj_particles.GetInt() > 0)
        {
            const char *pszEffect = "ExplosionCore_sapperdestroyed";

            if (bIsWater)
            {
                if (Q_strlen(pWeaponInfo->m_szExplosionWaterEffect) > 0)
                    pszEffect = pWeaponInfo->m_szExplosionWaterEffect;
            }
            else
            {
                if (bInAir)
                {
                    if (Q_strlen(pWeaponInfo->m_szExplosionMidAirEffect) > 0)
                        pszEffect = pWeaponInfo->m_szExplosionMidAirEffect;
                }
                else
                {
                    if (Q_strlen(pWeaponInfo->m_szExplosionEffect) > 0)
                        pszEffect = pWeaponInfo->m_szExplosionEffect;
                }
            }

            DispatchParticleEffect(pszEffect, m_vecOrigin, angExplosion);
        }
    }
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TETFExplosion, DT_TETFExplosion, CTETFExplosion)
    RecvPropVector(RECVINFO(m_vecOrigin)),
    RecvPropVector(RECVINFO(m_vecNormal)),
    RecvPropInt(RECVINFO(m_iWeaponID)),
END_RECV_TABLE();