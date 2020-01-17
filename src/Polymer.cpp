#include "Polymer.h"

Polymer::Polymer()
{
}

Polymer::Polymer(int noOfMonomers,
		 std::string sequence,
		 bool random,
                 bool fixedStart,
                 Vector3D start,
                 Vector3D orientation,
                 std::vector<Particle *> * particles)
{
  this->noOfMonomers = noOfMonomers;
  
  chain.resize(noOfMonomers);
  
  //int counter = 0;
  //int max_counter = 36;
  
  // calculate initial positions and create Particle array
  for (int i = noOfMonomers - 1; i > -1; i--)
    {
      Vector3D position;
      
      bool fixed = false;
      if (i==0 && fixedStart)
	fixed = true;

      Particle * next = NULL;
      if (i < noOfMonomers - 1)
	next = &chain[i+1];

      // Particles arranged in a straight line
      if (WorldSettings::initialCondition == WorldSettings::IC_LINE) {
	position = start + i * WorldSettings::bondLength * orientation;
      } else {
	if ( i == noOfMonomers - 1 ) {
	  position = start; ///< \todo start is position of last monomer
	} else {
	  // Self Avoiding Walk
	  /// \todo check for correctness after rewrite
	  /// \todo reimplement counter for impossible configurations
	  if (WorldSettings::initialCondition == WorldSettings::IC_SAW) {
	    bool valid;
	    do {
	      valid = true;
	      Vector3D step = noiseTerm(1);
	      position = chain[i+1].r +
		WorldSettings::bondLength * step / step.magnitude();

	      for (int j = 2 ; valid && j + i < noOfMonomers ; j++)
		if (distanceCheck(chain[i + j].r - position))
		  valid = false;
	    } while (!valid);
	  }
	  
	  // Random Walk
	  if (WorldSettings::initialCondition == WorldSettings::IC_RW) {
	    Vector3D step = noiseTerm(1);
	    position = chain[i+1].r +
	      WorldSettings::bondLength * step / step.magnitude();
	  }
	}
      }
      chain[i] = Particle(position,
			  AMINO_ACID,
			  sequence[i],
			  next,
			  fixed);
    }
  for(int i = 0; i < noOfMonomers; i++)
    {  
      particles -> push_back(&(chain[i])); 
    }
}

Polymer::~Polymer()
{
}

TYPE_FLOAT Polymer::endToEndDistanceSquared()
{
  Vector3D diff = chain[0].r - chain[noOfMonomers-1].r;
  return diff*diff;
}

Vector3D Polymer::endToEndVector()
{
  if (noOfMonomers == 1)
    return Vector3D();
  return chain[noOfMonomers-1].r - chain[0].r;
}

Vector3D Polymer::centreOfMass()
{
  Vector3D rCOM;
  for (int i = 0; i < noOfMonomers; i++)
    {
      rCOM += chain[i].r;
    }
  return rCOM/noOfMonomers;
}

TYPE_FLOAT Polymer::radiusOfGyrationSquared()
{
  TYPE_FLOAT rGsquared = 0;
  Vector3D rCOM = centreOfMass();
  
  for (int i = 0; i < noOfMonomers; i++)
    {
      rGsquared += (chain[i].r - rCOM)*(chain[i].r - rCOM);
    }
  return rGsquared/noOfMonomers;
}

/// \todo duplicate function in World
TYPE_FLOAT Polymer::averageBondLengthSquared()
{
  if (noOfMonomers == 1)
    return 0;
  Vector3D avg_b;
  TYPE_FLOAT avg_b_sq = 0;
  for (int i = 1; i < noOfMonomers; i++)
    {
      avg_b += (chain[i].r - chain[i-1].r);
      avg_b_sq += (chain[i].r - chain[i-1].r)*(chain[i].r - chain[i-1].r);
    }
  avg_b = avg_b/(noOfMonomers - 1);
  avg_b_sq /= (noOfMonomers - 1);
  return avg_b_sq;// - avg_b*avg_b;
}

// check for fixed particle occurs in World, when positions are updated
void Polymer::simulate(TYPE_FLOAT t)
{
  if (noOfMonomers > 1)
    {
      chain[0].dr += forceBond(chain[0].r - chain[1].r);
      for (int i = 1 ; i < noOfMonomers - 1 ; i++)
	{
	  chain[i].dr += forceBond(chain[i].r - chain[i+1].r) 
	    + forceBond(chain[i].r - chain[i-1].r);
        }
      chain[noOfMonomers - 1].dr += forceBond(chain[noOfMonomers - 1].r
					      - chain[noOfMonomers - 2].r);
    }
}

/// \todo should use Particle LJ radii
bool Polymer::distanceCheck(Vector3D r)
{
  TYPE_FLOAT r_mag_squared = r.magnitudeSquared();
  TYPE_FLOAT frac_bondlength_squared = WorldSettings::bondLength
    * WorldSettings::bondLength;
  if (r_mag_squared > frac_bondlength_squared)
    return false;
  return true;
}

void Polymer::draw(TYPE_FLOAT scale)
{
#ifdef GL
  //draw lines between the monomers
  glBegin(GL_LINE_STRIP);
  for (int i = 0; i < noOfMonomers; i++)
    {
      glVertex3f(chain[i].r.x/scale, chain[i].r.y/scale, chain[i].r.z/scale);
    }
  glEnd();
#endif
}
