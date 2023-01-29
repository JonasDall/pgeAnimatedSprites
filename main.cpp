#include <iostream>
#include <string>

#define OLC_PGE_APPLICATION
#include "lib/olcPixelGameEngine.hpp"

#define FRAMETIME 0.1f

class DecalAnim
{
private:
    olc::Decal* m_decal;
    olc::vi2d m_size;
    int m_maxFrame;
    int m_currentFrame = 0;
    float m_frametime = 0;

public:
    DecalAnim(olc::Decal* decal, int frames) : m_decal{decal}, m_size{(m_decal->sprite->width / (frames + 1)), (m_decal->sprite->height)}, m_maxFrame{frames}{}

    bool Update(float fElapsedTime)
    {
        if ((m_frametime += fElapsedTime) > FRAMETIME)
        {
            m_frametime -= FRAMETIME;
            if (++m_currentFrame > m_maxFrame)
            {
                m_currentFrame = 0;
                return true;
            }
        }

        return false;
    }

    void reset()
    {
        m_currentFrame = 0;
        m_frametime = 0;
    }

    olc::Decal* getDecal(){return m_decal;}

    olc::vi2d getSize(){return m_size;}

    unsigned int getFrame(){return m_currentFrame;}

    unsigned int getMaxFrame(){return m_maxFrame;}
};

class AnimState
{
private:
    DecalAnim* m_animation;
    bool m_loop;
    std::string m_nextAnim;

public:
    AnimState(DecalAnim* anim, bool isLooping, std::string nextAnim) : m_animation{anim}, m_loop{isLooping}, m_nextAnim{nextAnim}{}
    ~AnimState(){delete m_animation;}

    std::string Update(float fElapsedTime)
    {
        if (m_animation->Update(fElapsedTime) && !m_loop)
        {
            m_animation->reset();
            return m_nextAnim;
        }

        return "";
    }

    DecalAnim* getAnimation(){return m_animation;}

};

class StateMachine
{
private:
    std::unordered_map<std::string, AnimState*> m_states;
    std::vector<std::string> m_availableStates;
    std::vector<std::string> m_stateNames;
    std::string m_currentState;

public:
    StateMachine(){}
    ~StateMachine()
    {
        for (unsigned int i{}; i < m_stateNames.size(); ++i)
        {
            delete m_states.at(m_stateNames[i]);
        }
    }

    bool AddState(std::string stateName, olc::Decal* decal, int frames, bool isLooping, std::string nextAnim, bool availableState)
    {
        if ((m_states.count(stateName) > 0) || (stateName == "")) return 0;

        AnimState* construct = new AnimState(new DecalAnim{decal, frames}, isLooping, nextAnim);

        m_states[stateName] = construct;
        if (availableState) m_availableStates.push_back(stateName);

        m_stateNames.push_back(stateName);

        return 1;
    }

    bool SetState(std::string stateName)
    {
        if ((m_states.count(stateName) > 0) && !(stateName == m_currentState))
        {
            m_currentState = stateName;
            // std::cout << "State change to " << stateName << " success\n";
            return 1;
        }

        return 0;
    }

    bool Update(float fElapsedTime)
    {
        // std::string returnString = m_currentState->Update(fElapsedTime);
        std::string returnString = m_states.at(m_currentState)->Update(fElapsedTime);


        if (returnString != "")
        {
            if (!SetState(returnString))
            {
                // std::cout << "String not found\n";
                return 0;
            }
        }

        return 1;
    }

    DecalAnim* getAnimation()
    {
        return m_states[m_currentState]->getAnimation();
    }
};

//PGE inherit
class AnimationTest : public olc::PixelGameEngine
{
    // Private variables
private:
    olc::Sprite* m_idleSprite;
    olc::Sprite* m_runningSprite;
    olc::Sprite* m_jumpStartSprite;
    olc::Sprite* m_jumpLoopSprite;
    olc::Sprite* m_jumpEndSprite;

    olc::Decal* m_idleDecal;
    olc::Decal* m_runningDecal;
    olc::Decal* m_jumpStartDecal;
    olc::Decal* m_jumpLoopDecal;
    olc::Decal* m_jumpEndDecal;

    DecalAnim* m_anim;

    StateMachine* m_state;

    bool m_onFloor = false;
    bool m_running = false;
    bool m_jumping = false;
    bool m_flip = false;

    float m_gravity = 0.1;
    float m_floor = 70;
    float m_walkspeed = 1;
    float m_jumpheight = -0.05;
    olc::vf2d m_pos = {130, 70};
    olc::vf2d m_vel = {0.0, 0.0};

    // Public functions
public:

    AnimationTest()
    {
        sAppName = "Animation";
    }

    ~AnimationTest()
    {
        delete m_idleSprite;
        delete m_runningSprite;
        delete m_jumpStartSprite;
        delete m_jumpLoopSprite;
        delete m_jumpEndSprite;

        delete m_idleDecal;
        delete m_runningDecal;
        delete m_jumpStartDecal;
        delete m_jumpLoopDecal;
        delete m_jumpEndDecal;

        delete m_anim;

    }

    bool OnUserCreate() override
    {
        m_idleSprite = new olc::Sprite("./resources/IdleMan.png");
        m_runningSprite = new olc::Sprite("./resources/RunningMan.png");
        m_jumpStartSprite = new olc::Sprite("./resources/JumpingManStart.png");
        m_jumpLoopSprite = new olc::Sprite("./resources/JumpingManLoop.png");
        m_jumpEndSprite = new olc::Sprite("./resources/JumpingManEnd.png");

        // std::cout << "Created sprites\n";

        m_idleDecal = new olc::Decal(m_idleSprite);
        m_runningDecal = new olc::Decal(m_runningSprite);
        m_jumpStartDecal = new olc::Decal(m_jumpStartSprite);
        m_jumpLoopDecal = new olc::Decal(m_jumpLoopSprite);
        m_jumpEndDecal = new olc::Decal(m_jumpEndSprite);

        // std::cout << "Created decals\n";

        // m_anim = new DecalAnim(m_idleDecal, 4);
        m_state = new StateMachine();

        // std::cout << "Created state\n";

        m_state->AddState("Idle", m_idleDecal, 3, true, "Idle", true);
        m_state->AddState("Running", m_runningDecal, 4, true, "Running", true);
        m_state->AddState("JumpingStart", m_jumpStartDecal, 2, false, "JumpingLoop", true);
        m_state->AddState("JumpingLoop", m_jumpLoopDecal, 1, true, "JumpingLoop", false);
        m_state->AddState("JumpingEnd", m_idleDecal, 1, false, "Idle", false);

        // std::cout << "Added states\n";

        m_state->SetState("Idle");

        // std::cout << "Set state to Idle\n";

        return 1;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        if (GetKey(olc::Key::ESCAPE).bPressed)
            return 0;

        m_running = false;
        m_jumping = false;

        if (GetKey(olc::Key::D).bHeld)
        {
        m_vel.x += m_walkspeed * fElapsedTime;
        m_running = true;
        m_flip = false;
        }

        if (GetKey(olc::Key::A).bHeld)
        {
        m_vel.x += -m_walkspeed * fElapsedTime;
        m_running = true;
        m_flip = true;
        }

        if ((GetKey(olc::Key::SPACE).bPressed) && m_onFloor)
        {
            m_vel.y = m_jumpheight;
            // std::cout << m_vel.y << "\n";
            m_jumping = true;
        }

        m_vel.y += m_gravity * fElapsedTime;
        m_pos += m_vel;
        m_vel.x *= 0.99;

        if (m_pos.y >= m_floor)
        {
            m_vel.y = 0;
            m_pos.y = 70;

            // std::cout << m_onFloor << '\n';

            if (!m_onFloor)
            {
                m_state->SetState("JumpingEnd");
                // std::cout << "End jumping\n";
            }

            m_onFloor = true;
        }
        else
        m_onFloor = false;

        if (m_pos.x > 256 - 16)
            m_pos.x = 256 - 16;

        if (m_pos.x < 0)
            m_pos.x = 0;

        // m_anim->Update(fElapsedTime);

        if (m_jumping)
        {
            // std::cout << "Jump!\n";
            m_state->SetState("JumpingStart");
        }
        else
            if (m_onFloor)
            {
                if (m_running)
                    m_state->SetState("Running");
                else
                    m_state->SetState("Idle");
            }
            
        m_state->Update(fElapsedTime);

        olc::vi2d finalPosition{};
        olc::vi2d finalSize{};

        if (m_flip)
        {
            finalPosition.x = (int)m_pos.x + 16;
            finalPosition.y = (int)m_pos.y;

            finalSize.x = -(m_state->getAnimation()->getSize().x);
            finalSize.y = (m_state->getAnimation()->getSize().y);
        }
        else
        {
            finalPosition = (olc::vi2d)m_pos;

            finalSize = (olc::vi2d)m_state->getAnimation()->getSize();
        }

        // Clear({200, 250, 230});
        Clear({50, 50, 50});
        DrawLine({0, (int)m_floor + 16}, {256, (int)m_floor + 16}, olc::BLACK);
        // DrawDecal({0.0, 0.0}, m_decal.get());
        // DrawPartialDecal((olc::vi2d)m_pos, m_anim->getSize(), m_anim->getDecal(), {16 * (float)m_anim->getFrame(), 0}, {16, 16}, olc::WHITE);
        DrawPartialDecal(
            finalPosition,                                          //Position
            finalSize,                                              //Size
            m_state->getAnimation()->getDecal(),                    //Decal
            {16 * (float)m_state->getAnimation()->getFrame(), 0},   //Frame
            {16, 16},                                               //Out size
            olc::WHITE);                                            //Tint

        return 1;
    }
};

int main()
{
    AnimationTest object;
    if (object.Construct(256, 140, 4, 4))
        object.Start();

    return 0;
}