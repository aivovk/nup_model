#include "Force.h"

/* General force selector functions */
Vector3D forceBond(const Vector3D &r)
{
  if (WorldSettings::typeForceBond == WorldSettings::BOND_HOOKE)
    return forceBondHooke(r);
  else if (WorldSettings::typeForceBond == WorldSettings::BOND_EXP)
    return forceBondExp(r);
  return forceBondFENE(r);
}

Vector3D forceCohesive(const Vector3D &r, TYPE_FLOAT size)
{
  if (WorldSettings::typeForceHydrophobic == WorldSettings::HYDROPHOBIC_NORMAL)
    return forceLJ86Attractive(r, size);
  return forceLJ86AttractiveSameRange(r, size);
}

Vector3D forceRepulsive(const Vector3D &r, TYPE_FLOAT size)
{
  if (WorldSettings::typeForceRepulsive == WorldSettings::REPULSIVE_NORMAL)
    return forceLJ86Repulsive(r, size);
  return forceLJ126Repulsive(r, size);
}

Vector3D forceCharge(const Vector3D &r, TYPE_FLOAT charge)
{
  if (WorldSettings::typeForceCharge == WorldSettings::COULOMB)
    return forceChargeCoulomb(r, charge);
  return forceChargeDebye(r, charge);
}

/* implementations */

/* Bond forces */
Vector3D forceBondHooke(const Vector3D &r)
{
  return -r;
}

Vector3D forceBondFENE(const Vector3D &r)
{
  WorldSettings::totalCountFENE++;
  TYPE_FLOAT magSquared = r.magnitudeSquared();
  if (magSquared >= WorldSettings::maxFENELengthSquared)
    {
      WorldSettings::errorCountFENE++;
      return forceBondHooke(r);
    }
  return -r/(1-magSquared/WorldSettings::maxFENELengthSquared);
}

Vector3D forceBondExp(const Vector3D &r)
{
  return -r*exp(r.magnitudeSquared()/WorldSettings::maxFENELengthSquared);
}

/*** repulsive forces ***/
Vector3D forceExpRepulsive(const Vector3D &r, TYPE_FLOAT size)
{
  TYPE_FLOAT r_mag = r.magnitude();
  TYPE_FLOAT cutoff = 0.512 * WorldSettings::bondLength;
  TYPE_FLOAT beta = 4 * WorldSettings::bondLength;
  if (r_mag > cutoff)
    return Vector3D();
  TYPE_FLOAT forceMagnitude = 0.5 * 75.0 * beta * exp( - beta * r_mag );
  Vector3D force = r * forceMagnitude / r_mag;
  return force;
}

// !!! size is in BL units
Vector3D forceLJ86Repulsive(const Vector3D &r, TYPE_FLOAT size)
{
  TYPE_FLOAT r_mag_squared = r.magnitudeSquared();
  TYPE_FLOAT c_squared = 3*size*size; //sqrt(3) if b_LJ = b_Spring
  if (r_mag_squared > c_squared / 2) //b_LJ
    return Vector3D();

  TYPE_FLOAT forceMagnitude = 0.5*WorldSettings::eLJ
    *(0.5*pow(c_squared/r_mag_squared, 4) - pow(c_squared/r_mag_squared,3));
  
  WorldSettings::totalCountLJ++;
  
  Vector3D force = r * forceMagnitude/r_mag_squared;
    
  return force;
}

Vector3D forceLJ96Repulsive(const Vector3D &r, TYPE_FLOAT size)
{
  TYPE_FLOAT r_mag_squared = r.magnitudeSquared();
  TYPE_FLOAT r6 = r_mag_squared * r_mag_squared*r_mag_squared;
  TYPE_FLOAT r3 = pow(r_mag_squared, 1.5);
  if (r_mag_squared > 1.5)
    return Vector3D();

  TYPE_FLOAT forceMagnitude = WorldSettings::eLJ
    *(27*sqrt(1.5)/r3-18)/r6;
  WorldSettings::totalCountLJ++;
  Vector3D force = r * forceMagnitude/r_mag_squared;
  
  return force;
}

Vector3D forceLJ126Repulsive(const Vector3D &r, TYPE_FLOAT size)
{
  TYPE_FLOAT r_mag_squared = r.magnitudeSquared();
  TYPE_FLOAT c_squared = 3*size*size; //sqrt(3) if b_LJ = b_Spring
  if (r_mag_squared > c_squared / 2) //b_LJ
    return Vector3D();

  TYPE_FLOAT cr3 = pow(c_squared/r_mag_squared,3);
  TYPE_FLOAT forceMagnitude = 0.75*WorldSettings::eLJ
    *(0.125*pow(cr3, 2) - cr3);
  
  WorldSettings::totalCountLJ++;
  
  Vector3D force = r * forceMagnitude/r_mag_squared;
  return force;
}

/*** hydrophobic / cohesive forces ***/
//!!! size is in BL units
Vector3D forceLJ86Attractive(const Vector3D &r, TYPE_FLOAT size)
{
  TYPE_FLOAT r_mag_squared = r.magnitudeSquared();
  TYPE_FLOAT c_squared = 3*size*size;
  if (r_mag_squared < c_squared / 2 || r_mag_squared > size*size*WorldSettings::bondLength * WorldSettings::bondLength * 
      WorldSettings::hydrophobicCutoff * WorldSettings::hydrophobicCutoff)
    return Vector3D();
  TYPE_FLOAT forceMagnitude = 0.5*WorldSettings::hydrophobicStrength
    *(0.5*pow(c_squared/r_mag_squared, 4) - pow(c_squared/r_mag_squared,3));

  Vector3D force = r * forceMagnitude/r_mag_squared;

  return force;
}

Vector3D forceLJ126Attractive(const Vector3D &r, TYPE_FLOAT size)
{
  TYPE_FLOAT r_mag_squared = r.magnitudeSquared();
  TYPE_FLOAT c_squared = 3*size*size;
  //note farther cutoff depends on size
  //csCohesive is based on max size
  if (r_mag_squared < c_squared / 2 || r_mag_squared > WorldSettings::bondLength * WorldSettings::bondLength * size * size * WorldSettings::hydrophobicCutoff * WorldSettings::hydrophobicCutoff)
    return Vector3D();
  TYPE_FLOAT cr3 = pow(c_squared/r_mag_squared,3);
  TYPE_FLOAT forceMagnitude = 0.75*WorldSettings::hydrophobicStrength
    *(0.125*pow(cr3, 2) - cr3);
  
  Vector3D force = r * forceMagnitude/r_mag_squared;
  
  return force;
  
}

Vector3D forceLJ86AttractiveSameRange(const Vector3D &r, TYPE_FLOAT size)
{
  TYPE_FLOAT r_mag_squared = r.magnitudeSquared() + 
    (1-size)*(1-size)*1.5 + 
    2*r.magnitude()*(1-size)*WorldSettings::bondLength;
  TYPE_FLOAT c_squared = 3;
  if (r_mag_squared < c_squared / 2|| r_mag_squared > WorldSettings::bondLength * WorldSettings::bondLength * 
      WorldSettings::hydrophobicCutoff * WorldSettings::hydrophobicCutoff)  
    return Vector3D();
  TYPE_FLOAT forceMagnitude = 0.75*WorldSettings::hydrophobicStrength
    *(0.5*pow(c_squared/r_mag_squared, 4) - pow(c_squared/r_mag_squared,3));

  Vector3D force = r * forceMagnitude/r_mag_squared;

  return force;
}

/*** charged forces ***/
Vector3D forceChargeCoulomb(const Vector3D &r, TYPE_FLOAT charge)
{ 
  return WorldSettings::coulombStrength * charge * r / (r.magnitude() * r.magnitudeSquared());
}

Vector3D forceChargeDebye(const Vector3D &r, TYPE_FLOAT charge)
{
  TYPE_FLOAT l = WorldSettings::debyeLength;
  
  return WorldSettings::coulombStrength * charge * r / r.magnitude() * (1 / r.magnitudeSquared() + 1 / (l * r.magnitude())) * exp( - r.magnitude()/l); ///< \todo explain
}


/*** random forces ***/

Vector3D noiseTerm(TYPE_FLOAT stddev)
{
  return Vector3D(stddev * WorldSettings::normalDistribution->generateUnitNormal(),
		  stddev * WorldSettings::normalDistribution->generateUnitNormal(),
		  stddev * WorldSettings::normalDistribution->generateUnitNormal());
}

Vector3D noiseTerm()
{
  return noiseTerm(WorldSettings::sqrt_dt);
}


/*** external forces ***/

Vector3D forceExternal(const Vector3D &r, TYPE_FLOAT size)
{
  if (WorldSettings::typeForceExternal == WorldSettings::EXT_ZWALL)
    return forceExternalZWall(r, size);
  return Vector3D();
}

Vector3D forceExternalZWall(const Vector3D &r, TYPE_FLOAT size)
{
  
  return forceRepulsive(r - Vector3D(r.x, r.y, WorldSettings::offset + 200 * WorldSettings::bondLength), size)
    + forceRepulsive(r - Vector3D(r.x, r.y, WorldSettings::offset), size);
}


