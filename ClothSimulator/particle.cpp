////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - particle.cpp
////////////////////////////////////////////////////////////////////////////////////////

#include "particle.h"
#include "shader.h"

namespace
{
    const float PARTICLE_VISUAL_RADIUS = 0.15f; ///< Visual draw radius of the particle
    const float PARTICLE_MASS = 1.0f;           ///< Mass in kg for single particle
}

Particle::Particle(EnginePtr engine) :
    m_acceleration(0.0f, 0.0f, 0.0f),
    m_previousPosition(0.0f, 0.0f, 0.0f),
    m_initialPosition(0.0f, 0.0f, 0.0f),
    m_position(0.0f, 0.0f, 0.0f),
    m_resetTranslation(0.0f, 0.0f, 0.0f),
    m_savedPosition(0.0f, 0.0f, 0.0f),
    m_savedPreviousPosition(0.0f, 0.0f, 0.0f),
    m_uvs(0.0f, 0.0f),
    m_selected(false),
    m_pinned(false),
    m_index(NO_INDEX),
    m_resetMotion(false)
{
    auto resolveCollisionFn = std::bind(&Particle::MovePosition, this, std::placeholders::_1);
    auto resetMotionFn = std::bind(&Particle::ChangeMotion, this, std::placeholders::_1);

    m_collision.reset(new CollisionMesh(m_transform, engine));
    m_collision->MakeDynamic(resetMotionFn, resolveCollisionFn);
    m_collision->SetDraw(true);
    
    Transform::UpdateFn fullFn = std::bind(&CollisionMesh::FullUpdate, m_collision.get());
    Transform::UpdateFn positionalFn = std::bind(&CollisionMesh::PositionalUpdate, m_collision.get());
    m_transform.SetObserver(fullFn, positionalFn);
}

void Particle::Initialise(const D3DXVECTOR3& position, const D3DXVECTOR2& uv, unsigned int index,
    std::shared_ptr<CollisionMesh::Geometry> geometry, const CollisionMesh::Data& data)
{
    m_uvs = uv;
    ResetAcceleration();
    m_initialPosition = position;
    m_position = position;
    m_previousPosition = position;
    m_index = index;
    m_transform.SetPosition(m_position);
    m_collision->LoadInstance(data, geometry);
}

void Particle::ChangeMotion(const D3DXVECTOR3& motion)
{
    m_resetMotion = true;
    m_resetTranslation += motion;
}

void Particle::ResetPosition()
{
    m_previousPosition = m_position = m_initialPosition;
    m_savedPosition = m_previousPosition = m_initialPosition;
    m_transform.SetPosition(m_position);
}

void Particle::ResetAcceleration()
{ 
    m_acceleration.x = 0.0f;
    m_acceleration.y = 0.0f;
    m_acceleration.z = 0.0f;
}

bool Particle::IsColliding() const
{
    return m_collision->IsColliding();
}

void Particle::PinParticle(bool pin)
{
    m_pinned = pin;
}

void Particle::SelectParticle(bool select)
{
    m_selected = select;
}

void Particle::DrawVisualMesh(const Matrix& projection, const Matrix& view)
{
    m_collision->DrawWithRadius(projection, view, PARTICLE_VISUAL_RADIUS);
}

void Particle::DrawCollisionMesh(const Matrix& projection, const Matrix& view)
{
    m_collision->DrawMesh(projection, view);
}

CollisionMesh& Particle::GetCollisionMesh()
{
    return *m_collision;
}

void Particle::SetColor(const D3DXVECTOR3& colour)
{
    m_collision->SetColor(colour);
}

void Particle::MovePosition(const D3DXVECTOR3& position)
{
    if(!m_pinned)
    {
        m_position += position;
        m_transform.SetPosition(m_position);
    }
};

void Particle::AddForce(const D3DXVECTOR3& force)
{    
    if(!m_pinned)
    {
        m_acceleration += force/PARTICLE_MASS;
    }
}

void Particle::PreCollisionUpdate(float damping, float timestepSqr)
{
    if(!m_pinned)
    {
        //verlet integration
        //X(t + ∆t) = 2X(t) - X(t - ∆t) + ∆t^2X▪▪(t)
        //X(t + ∆t) = X(t) + (X(t)-X(t - ∆t)) + ∆t^2X▪▪(t)
        //X(t + ∆t) = X(t) + X▪(t) + ∆t^2X▪▪(t)
        m_position += ((m_position-m_previousPosition)*damping) + (m_acceleration*timestepSqr);
        
        m_previousPosition.x = m_transform.GetMatrix()._41;
        m_previousPosition.y = m_transform.GetMatrix()._42;
        m_previousPosition.z = m_transform.GetMatrix()._43;
        m_transform.SetPosition(m_position);

        ResetAcceleration();
        m_resetTranslation.x = 0.0f;
        m_resetTranslation.y = 0.0f;
        m_resetTranslation.z = 0.0f;
    }
}

void Particle::PostCollisionUpdate()
{
    if(m_resetMotion)
    {
        m_resetMotion = false;
        m_position = m_resetTranslation + m_savedPosition;
        m_previousPosition = m_savedPreviousPosition;
    }
    else
    {
        m_savedPosition = m_position;
        m_savedPreviousPosition = m_previousPosition;
    }
    m_transform.SetPosition(m_position);
    m_collision->UpdateCollision();
}