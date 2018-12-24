#include "stdafx.h"
#include <gl/GL.h>
#include "Engine/Error.h"
#include "Game.h"
#include "Panel.h"
#include "Actor.h"
#include "Skills.h"

namespace Game
{

    
    Button::Button(const Skill& skill) :
        highlighted(false),
        skill(skill)
    {
        texture.Load(std::wstring(L"Data/") + skill.name + std::wstring(L".png"));
    }

    Button::Button(Button&& other) :
        skill(other.skill),
        texture(std::move(other.texture)),
        highlighted(other.highlighted)
    {

    }

    void Button::Highlight(bool on)
    {
        highlighted = on;
    }
    void Button::Render() const
    {
        glPushName(GLuint(skill.Id()));

        texture.Bind();
        if (highlighted)
            glColor3f(1.0f, 1.0f, 1.0f);
        else
            glColor3f(0.1f, 0.1f, 0.1f);
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 1.0f);
            glVertex2f(0.0f, 0.0f);

            glTexCoord2f(1.0f, 1.0f);
            glVertex2f(1.0f, 0.0f);

            glTexCoord2f(1.0f, 0.0f);
            glVertex2f(1.0f, 1.0f);

            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(0.0f, 1.0f);
        glEnd();
        texture.Unbind();
        glPopName();
    }

    Panel::Panel(Game& game, unsigned height) :
        game(game),
        height(height),
        actor(nullptr),
        actorConnection(game.actorActivated.connect([this](Actor* actor)
        {
            UpdateSkills(actor);
        })),
        skillConnection(game.skillSelected.connect([this](const Skill* skill)
        {
            HighlightSkill(skill);
        }))
    {
    }

    unsigned Panel::Height() const
    {
        return height;
    }

    void Panel::Render() const
    {
        glDisable(GL_LIGHTING);
        glMatrixMode(GL_PROJECTION);
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glOrtho(0, viewport[2], 0, viewport[3], -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glScalef(float(height), float(height), 1.0f);

        glPushName(GLuint(Game::Selection::Skill));
        for (auto& button : buttons)
        {
            button.Render();
            glTranslatef(1.0f, 0.0f, 0.0f);
        }
        glPopName();
        Engine::CheckGLError();
    }

    void Panel::Key(unsigned short code)
    {
        if ((code >= VK_F1) && (code <= VK_F24))
        {
            unsigned button = code - VK_F1;
            if ((actor) && (button < buttons.size()))
            {
                game.SelectSkill(&buttons.at(button).skill);
            }
        }
    }

    void Panel::UpdateSkills(Actor* newActor)
    {
        actor = newActor;
        buttons.clear();
        if (actor)
        {
            for (auto skill : actor->GetSkills())
            {
                if (skill.skill->IsActive())
                {
                    buttons.emplace_back(Button(*skill.skill));
                }
                if (skill.skill == game.SelectedSkill())
                {
                    buttons.back().Highlight(true);
                }
            }
        }
    }

    void Panel::HighlightSkill(const Skill* skill)
    {
        for (auto& button : buttons)
        {
            if (&button.skill == skill)
            {
                button.Highlight(true);
            }
            else
            {
                button.Highlight(false);
            }
        }
    }
};