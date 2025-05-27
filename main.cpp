#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <unordered_map>

#define USE_PEGAFOX_UTILS_IMPLEMENTATION
#include <pegafox/utils.hpp>

#include <glm/gtx/rotate_vector.hpp>

#include "tile.hpp"

typedef std::unordered_map<glm::i16vec2, std::unique_ptr<Tile>, decltype([](const glm::i16vec2& vec){return (vec.y << 16) | vec.x;})> World;

#include "turtle.hpp"

World editorWorld;
World programWorld;

int main()
{
  pf::FPS fpsClock;
  
  Tile::texture.loadFromFile("../tiles.png");
  Tile::texture.setSmooth(true);

  editorWorld[glm::i16vec2(0, 0)] = std::unique_ptr<Tile>(new Tile(Tile::Gem));
  editorWorld[glm::i16vec2(-1, 0)] = std::unique_ptr<Tile>(new Tile(Tile::StoneWall));
  editorWorld[glm::i16vec2(-1, -1)] = std::unique_ptr<Tile>(new Tile(Tile::StoneWall));
  editorWorld[glm::i16vec2(0, -1)] = std::unique_ptr<Tile>(new Tile(Tile::StoneWall));

  sf::VertexArray lines(sf::PrimitiveType::Lines, 2);
  lines[0].color = sf::Color(128, 128, 128+32);
  lines[1].color = sf::Color(128, 128, 128+32);

  Turtle* selectedTurtle = nullptr;

  Tile::Color currentColor = Tile::Green;

  Tile::Type currentTile = Tile::Turtle;

  Turtle::Instruction currentInstruction = Turtle::Left;

  bool settingsOpen = false;
  bool simulating = false;

  sf::RectangleShape sidebar;

  sf::Font font;
  sf::Text text(font);

  sf::RenderWindow SCREEN(sf::VideoMode::getFullscreenModes()[0], "robot turtles", sf::State::Fullscreen);
  std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes();

  sf::View cam = SCREEN.getDefaultView();
  cam.move(-cam.getSize()*0.5f);
  SCREEN.setFramerateLimit(60);
  while (SCREEN.isOpen())
  {
    SCREEN.setView(cam);

    while (std::optional<sf::Event> event = SCREEN.pollEvent())
    {
      if (event->is<sf::Event::Closed>())
      {
        SCREEN.close();
      } else if (const sf::Event::MouseWheelScrolled* scroll = event->getIf<sf::Event::MouseWheelScrolled>())
      {
        if (selectedTurtle != nullptr && scroll->position.x < Tile::spriteSize*2 && scroll->position.y >= Tile::spriteSize*9)
        {
          if (scroll->position.x < Tile::spriteSize)
          {
            selectedTurtle->moveProgramOffset(-scroll->delta);
          } else
          {
            selectedTurtle->moveFunctionOffset(-scroll->delta);
          }
        } else
        {
          cam.zoom(1.0f - scroll->delta * 0.1f);
        }
      } else if (const sf::Event::MouseButtonPressed* press = event->getIf<sf::Event::MouseButtonPressed>())
      {
        if (press->position.x > Tile::spriteSize*2)
        {
          glm::i16vec2 pos = glm::i16vec2(glm::floor(SCREEN.mapPixelToCoords(sf::Vector2i(press->position.x, press->position.y)).x / Tile::spriteSize), glm::floor(SCREEN.mapPixelToCoords(sf::Vector2i(press->position.x, press->position.y)).y / Tile::spriteSize));

          if (editorWorld.contains(pos))
          {
            if (editorWorld[pos]->getType() == Tile::Turtle)
            {
              selectedTurtle = (Turtle*)editorWorld[pos].get();
            }

            if (press->button == sf::Mouse::Button::Right && programWorld.empty())
            {
              editorWorld[pos]->setDir(editorWorld[pos]->getDir() + 1);
            }
          }
        } else
        {
          glm::ivec2 selectedButton = glm::ivec2(press->position.x, press->position.y) / Tile::spriteSize;

          if (selectedButton == glm::ivec2(0, 0))
          {
            selectedTurtle = nullptr;
            programWorld.clear();
            simulating = false;
          } else if (selectedButton == glm::ivec2(1, 0))
          {
            if (!programWorld.empty())
            {
              for (std::pair<const glm::i16vec2, std::unique_ptr<Tile>>& tile: programWorld)
              {
                if (tile.second.get() != nullptr && tile.second->getType() == Tile::Turtle)
                {
                  ((Turtle*)tile.second.get())->stepBackward(tile.first, editorWorld, programWorld);
                }
              }
            }
          } else if (selectedButton == glm::ivec2(0, 1))
          {
            simulating = !simulating;
            if (simulating)
            {
              if (programWorld.empty())
              {
                for (std::pair<const glm::i16vec2, std::unique_ptr<Tile>>& tile: editorWorld)
                {
                  if (tile.second->getType() == Tile::Turtle)
                  {
                    programWorld[tile.first] = std::unique_ptr<Tile>(new Turtle(*((Turtle*)tile.second.get())));
                    if ((Turtle*)tile.second.get() == selectedTurtle)
                    {
                      selectedTurtle = (Turtle*)programWorld[tile.first].get();
                    }
                  } else
                  {
                    programWorld[tile.first] = std::unique_ptr<Tile>(new Tile(*tile.second));
                  }
                }
              }
            }
          } else if (selectedButton == glm::ivec2(1, 1))
          {
            if (!programWorld.empty())
            {
              for (std::pair<const glm::i16vec2, std::unique_ptr<Tile>>& tile: programWorld)
              {
                if (tile.second.get() != nullptr && tile.second->getType() == Tile::Turtle)
                {
                  ((Turtle*)tile.second.get())->stepForward(tile.first, programWorld);
                }
              }
            }
          } else if (sf::IntRect(sf::Vector2i(0, 2), sf::Vector2i(2, 2)).contains(sf::Vector2i(selectedButton.x, selectedButton.y)))
          {
            currentColor = Tile::Color((selectedButton.y-2)*2 + selectedButton.x);
          } else if (selectedButton == glm::ivec2(0, 4))
          {
            currentTile = Tile::Turtle;
          } else if (selectedButton == glm::ivec2(1, 4))
          {
            currentTile = Tile::Gem;
          } else if (selectedButton == glm::ivec2(0, 5))
          {
            currentTile = Tile::IceWall;
          } else if (selectedButton == glm::ivec2(1, 5))
          {
            currentTile = Tile::StoneWall;
          } else if (selectedButton == glm::ivec2(0, 6))
          {
            currentTile = Tile::Crate;
          } else if (sf::IntRect(sf::Vector2i(0, 6), sf::Vector2i(2, 3)).contains(sf::Vector2i(selectedButton.x, selectedButton.y)))
          {
            currentInstruction = Turtle::Instruction((selectedButton.y-6)*2 + selectedButton.x);
          } else if (selectedTurtle != nullptr && programWorld.empty())
          {
            if (press->position.x > Tile::spriteSize)
            {
              if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
              {
                selectedTurtle->eraseFromFunction(selectedButton.y-8 + selectedTurtle->getFunctionOffset());
              } else
              {
                selectedTurtle->insertToFunction(currentInstruction, selectedButton.y-9 + selectedTurtle->getFunctionOffset());
              }
            } else
            {
              if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
              {
                selectedTurtle->eraseFromProgram(selectedButton.y-8 + selectedTurtle->getProgramOffset());
              } else
              {
                selectedTurtle->insertToProgram(currentInstruction, selectedButton.y-9 + selectedTurtle->getProgramOffset());
              }
            }
          }
        }
      } else if (const sf::Event::KeyReleased* release = event->getIf<sf::Event::KeyReleased>())
      {
        if (release->code == sf::Keyboard::Key::Escape)
        {
          settingsOpen = !settingsOpen;
        }
      }
    }
    SCREEN.setTitle(std::to_string(int(fpsClock.get_fps())));

    if (SCREEN.hasFocus())
    {
      if (programWorld.empty() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && sf::Mouse::getPosition(SCREEN).x > Tile::spriteSize*2)
      {
        glm::i16vec2 pos(glm::floor(SCREEN.mapPixelToCoords(sf::Mouse::getPosition(SCREEN)).x / Tile::spriteSize), glm::floor(SCREEN.mapPixelToCoords(sf::Mouse::getPosition(SCREEN)).y / Tile::spriteSize));
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
        {
          if (editorWorld.contains(pos))
          {
            editorWorld.erase(pos);
          }
        } else
        {
          if (!editorWorld.contains(pos) || editorWorld[pos] == nullptr || editorWorld[pos]->getType() != currentTile || editorWorld[pos]->getColor() != currentColor)
          {
            if (currentTile == Tile::Turtle)
            {
              editorWorld[pos] = std::unique_ptr<Tile>(new Turtle(currentColor));
              selectedTurtle = (Turtle*)editorWorld[pos].get();
            } else
            {
              editorWorld[pos] = std::unique_ptr<Tile>(new Tile(currentTile, currentColor));
            }
          }
        }
      }

      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
      {
        cam.move(sf::Vector2f(0, -cam.getSize().y * 0.01f));
      }
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
      {
        cam.move(sf::Vector2f(0, cam.getSize().y * 0.01f));
      }
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
      {
        cam.move(sf::Vector2f(-cam.getSize().x * 0.01f, 0));
      }
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
      {
        cam.move(sf::Vector2f(cam.getSize().x * 0.01f, 0));
      }
    }

    if (simulating)
    {
      for (std::pair<const glm::i16vec2, std::unique_ptr<Tile>>& tile: programWorld)
      {
        if (tile.second.get() != nullptr && tile.second->getType() == Tile::Turtle)
        {
          ((Turtle*)tile.second.get())->stepForward(tile.first, programWorld);
        }
      }
    }

    SCREEN.clear(sf::Color(214, 184, 146));

    for (int x = -cam.getSize().x / Tile::spriteSize / 2; x < cam.getSize().x / Tile::spriteSize / 2; x++)
    {
      lines[0].position = sf::Vector2f((int(cam.getCenter().x/Tile::spriteSize) + x) * Tile::spriteSize, cam.getCenter().y - cam.getSize().y / 2);
      lines[1].position = sf::Vector2f((int(cam.getCenter().x/Tile::spriteSize) + x) * Tile::spriteSize, cam.getCenter().y + cam.getSize().y / 2);

      SCREEN.draw(lines);
    }

    for (int y = -cam.getSize().y / Tile::spriteSize / 2; y < cam.getSize().y / Tile::spriteSize / 2; y++)
    {
      lines[0].position = sf::Vector2f(cam.getCenter().x - cam.getSize().x / 2, (int(cam.getCenter().y/Tile::spriteSize) + y) * Tile::spriteSize);
      lines[1].position = sf::Vector2f(cam.getCenter().x + cam.getSize().x / 2, (int(cam.getCenter().y/Tile::spriteSize) + y) * Tile::spriteSize);

      SCREEN.draw(lines);
    }

    if (!programWorld.empty())
    {
      for (std::pair<const glm::i16vec2, std::unique_ptr<Tile>>& tile: programWorld)
      {
        if (tile.second.get() != nullptr)
        {
          tile.second->draw(SCREEN, tile.first);
        }
      }
    } else
    {
      for (std::pair<const glm::i16vec2, std::unique_ptr<Tile>>& tile: editorWorld)
      {
        tile.second->draw(SCREEN, tile.first);
      }
    }

    SCREEN.setView(SCREEN.getDefaultView());

    sidebar.setPosition(sf::Vector2f(0, 0));
    sidebar.setSize(sf::Vector2f(Tile::spriteSize*2, SCREEN.getSize().y));
    SCREEN.draw(sidebar);

    sidebar.setSize(sf::Vector2f(Tile::spriteSize, Tile::spriteSize));
    sidebar.setTexture(&Tile::texture);
    sidebar.setFillColor(sf::Color::Black);

    // reset
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*21, 0), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    SCREEN.draw(sidebar);

    // step back
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*21, Tile::spriteSize), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    sidebar.setPosition(sf::Vector2f(Tile::spriteSize, 0));
    SCREEN.draw(sidebar);

    // pause/play
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*22, Tile::spriteSize*simulating), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    sidebar.setPosition(sf::Vector2f(0, Tile::spriteSize));
    SCREEN.draw(sidebar);

    // step forward
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*23, Tile::spriteSize), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    sidebar.setPosition(sf::Vector2f(Tile::spriteSize, Tile::spriteSize));
    SCREEN.draw(sidebar);

    sidebar.setFillColor(sf::Color::White);

    // green
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*23, 0), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    sidebar.setPosition(sf::Vector2f(0, Tile::spriteSize*2));
    SCREEN.draw(sidebar);

    // blue
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*24, 0), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    sidebar.setPosition(sf::Vector2f(Tile::spriteSize, Tile::spriteSize*2));
    SCREEN.draw(sidebar);

    // pink
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*24, Tile::spriteSize), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    sidebar.setPosition(sf::Vector2f(0, Tile::spriteSize*3));
    SCREEN.draw(sidebar);

    // red
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*25, 0), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    sidebar.setPosition(sf::Vector2f(Tile::spriteSize, Tile::spriteSize*3));
    SCREEN.draw(sidebar);

    // turtle
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*3, Tile::spriteSize), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    sidebar.setPosition(sf::Vector2f(0, Tile::spriteSize*4));
    SCREEN.draw(sidebar);

    // gem
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*4, Tile::spriteSize), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    sidebar.setPosition(sf::Vector2f(Tile::spriteSize, Tile::spriteSize*4));
    SCREEN.draw(sidebar);

    // ice wall
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*13, Tile::spriteSize), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    sidebar.setPosition(sf::Vector2f(0, Tile::spriteSize*5));
    SCREEN.draw(sidebar);

    // stone wall
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*12, Tile::spriteSize), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    sidebar.setPosition(sf::Vector2f(Tile::spriteSize, Tile::spriteSize*5));
    SCREEN.draw(sidebar);

    // crate
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*8, Tile::spriteSize), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    sidebar.setPosition(sf::Vector2f(0, Tile::spriteSize*6));
    SCREEN.draw(sidebar);

    sidebar.setFillColor(sf::Color::Black);

    // turn left
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*25, Tile::spriteSize), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    sidebar.setPosition(sf::Vector2f(Tile::spriteSize, Tile::spriteSize*6));
    SCREEN.draw(sidebar);

    // walk forward
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*26, 0), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    sidebar.setPosition(sf::Vector2f(0, Tile::spriteSize*7));
    SCREEN.draw(sidebar);

    // turn right
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*26, Tile::spriteSize), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    sidebar.setPosition(sf::Vector2f(Tile::spriteSize, Tile::spriteSize*7));
    SCREEN.draw(sidebar);

    // use laser
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*27, 0), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    sidebar.setPosition(sf::Vector2f(0, Tile::spriteSize*8));
    SCREEN.draw(sidebar);

    // jump to frog
    sidebar.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize*27, Tile::spriteSize), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
    sidebar.setPosition(sf::Vector2f(Tile::spriteSize, Tile::spriteSize*8));
    SCREEN.draw(sidebar);

    if (selectedTurtle != nullptr)
    {
      selectedTurtle->drawProgram(SCREEN, sf::IntRect(sf::Vector2i(0, Tile::spriteSize*9), sf::Vector2i(Tile::spriteSize*2, SCREEN.getSize().y-Tile::spriteSize*9)));
    }

    sidebar.setTexture(nullptr);
    sidebar.setFillColor(sf::Color::Transparent);
    sidebar.setOutlineThickness(-3);
    
    sidebar.setOutlineColor(sf::Color(128, 128, 128));
    sidebar.setPosition(sf::Vector2f((int)currentColor%2 * Tile::spriteSize, Tile::spriteSize*2 + (int)currentColor/2 * Tile::spriteSize));
    SCREEN.draw(sidebar);

    sidebar.setOutlineColor(sf::Color::Blue);
    switch (currentTile)
    {
      case Tile::Gem:
        sidebar.setPosition(sf::Vector2f(Tile::spriteSize, Tile::spriteSize*4));
        break;
      case Tile::IceWall:
        sidebar.setPosition(sf::Vector2f(0, Tile::spriteSize*5));
        break;
      case Tile::StoneWall:
        sidebar.setPosition(sf::Vector2f(Tile::spriteSize, Tile::spriteSize*5));
        break;
      case Tile::Crate:
        sidebar.setPosition(sf::Vector2f(0, Tile::spriteSize*6));
        break;
      case Tile::Turtle:
        sidebar.setPosition(sf::Vector2f(0, Tile::spriteSize*4));
        break;
    }
    SCREEN.draw(sidebar);

    sidebar.setOutlineColor(sf::Color::Green);
    sidebar.setPosition(sf::Vector2f((int)currentInstruction%2 * Tile::spriteSize, Tile::spriteSize*6 + (int)currentInstruction/2 * Tile::spriteSize));
    SCREEN.draw(sidebar);

    sidebar.setOutlineThickness(0);
    sidebar.setFillColor(sf::Color::White);
    if (settingsOpen)
    {
      sidebar.setPosition(sf::Vector2f(SCREEN.getSize())*0.5f - sf::Vector2f(200, 200));
      sidebar.setSize(sf::Vector2f(400, 400));
      SCREEN.draw(sidebar);
    }

    SCREEN.display();
  }
  return 0;
}
