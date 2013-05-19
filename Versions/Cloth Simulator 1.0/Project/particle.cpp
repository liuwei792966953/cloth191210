
#include "particle.h"

//=============================================================================
//Initialise the particle
//=============================================================================
void PARTICLE::InitialiseParticle()
{
	pinned = false;
	initial_position = old_position = position;
}
//=============================================================================
//ADD A FORCE FOR PARTICLE
//=============================================================================
void PARTICLE::AddForce(FLOAT3 & force)
{	
	if(!pinned)
		acceleration += force/PARTICLE_MASS;
}
//=============================================================================
//PARTICLE TIMESTEP
//=============================================================================
void PARTICLE::OnTimeStep()
{
	//if the particle can move
	if(!pinned)
	{
		//store the current position
		FLOAT3 temp = position;

		//VERLET INTEGRATION
		//X(t + ∆t) = 2X(t) - X(t - ∆t) + ∆t^2X▪▪(t)
		//X(t + ∆t) = X(t) + (X(t)-X(t - ∆t)) + ∆t^2X▪▪(t)
		//X(t + ∆t) = X(t) + X▪(t) + ∆t^2X▪▪(t)
		position = position + ((position-old_position)*DAMPING) + (acceleration*TIMESTEP_SQUARED);

		//save the old position
		old_position = temp; 
		acceleration = FLOAT3(0,0,0);
	}
}
//=============================================================================
//SPRING CONSTRUCTOR
//=============================================================================
SPRING::SPRING(PARTICLE *pt1, PARTICLE *pt2) : p1(pt1),p2(pt2)
{
	//get vector between both particles
	FLOAT3 vec = pt1->position - pt2->position;

	//get distance of vector
	restDistance = vec.Length();
}
//=============================================================================
//SOLVE SPRING; check how much the spring has been stretched and fix
//=============================================================================
void SPRING::SolveSpring()
{
	//current vector from p1 to p2
	FLOAT3 currentVector = p2->position - p1->position;

	//current distance using vector
	float currentDistance = currentVector.Length();

	//'error' vector between p1 and p2 (need to minimise this)
	FLOAT3 errorVector = currentVector - ((currentVector/currentDistance)*restDistance); 

	//half of the error vector
	FLOAT3 errorVectorHalf = errorVector*0.5;
	
	//move the particles back into place
	p1->MovePosition(errorVectorHalf);
	p2->MovePosition(-errorVectorHalf);

}
