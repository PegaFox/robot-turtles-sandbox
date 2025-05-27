class Turtle: public Tile
{
  public:
    enum Instruction: uint8_t
    {
      None,
      Left,
      Up,
      Right,
      Laser,
      Frog
    };

    Turtle(Color color = Green)
    {
      this->color = color;
      type = Tile::Turtle;
      updateSprite();
    }

    void insertToProgram(Instruction value, std::size_t pos)
    {
      if (pos >= program.size())
      {
        program.push_back(value);
      } else
      {
        program.insert(program.begin()+pos, value);
      }
    }

    void insertToFunction(Instruction value, std::size_t pos)
    {
      if (pos >= function.size())
      {
        function.push_back(value);
      } else
      {
        function.insert(function.begin()+pos, value);
      }
    }

    void eraseFromProgram(std::size_t pos)
    {
      if (program.empty())
      {
        return;
      }

      if (pos == program.size())
      {
        program.pop_back();
      } else if (pos < program.size())
      {
        program.erase(program.begin()+pos);
      }
    }

    void eraseFromFunction(std::size_t pos)
    {
      if (function.empty())
      {
        return;
      }

      if (pos == function.size())
      {
        function.pop_back();
      } else if (pos < function.size())
      {
        function.erase(function.begin()+pos);
      }
    }

    void moveProgramOffset(std::size_t offset)
    {
      programOffset = glm::clamp((int)programOffset + (int)offset, 0, (int)program.size());
    }

    std::size_t getProgramOffset()
    {
      return programOffset;
    }

    void moveFunctionOffset(std::size_t offset)
    {
      functionOffset = glm::clamp((int)functionOffset + (int)offset, 0, (int)function.size());
    }

    std::size_t getFunctionOffset()
    {
      return functionOffset;
    }

    bool stepForward(glm::i16vec2 pos, World& world)
    {
      if (programCounter == program.size())
      {
        programCounter = -1;
      }

      if (functionCounter == function.size())
      {
        functionCounter = -1;
      }

      Instruction instruction;
      if (functionCounter == -1)
      {
        if (programCounter == -1)
        {
          return false;
        }
        instruction = program[programCounter++];
      } else
      {
        instruction = function[functionCounter++];
      }


      switch (instruction)
      {
        case Left:
          sprite.rotate(sf::degrees(-90));
          break;
        case Up: {
          glm::i16vec2 vel(glm::rotate(glm::vec2(0, -1), (float)getDir()*glm::half_pi<float>()));

          if (world[pos+vel].get() != nullptr && world[pos+vel]->getType() == Tile::Crate && (world[pos+vel+vel].get() == nullptr || world[pos+vel+vel]->getType() == Tile::None))
          {
            world[pos+vel].swap(world[pos+vel+vel]);
          }

          if (world[pos+vel].get() == nullptr || world[pos+vel]->getType() == Tile::None)
          {
            world[pos].swap(world[pos+vel]);
            pos += vel;
          }
          break;
        } case Right:
          sprite.rotate(sf::degrees(90));
          break;
        case Laser: {
          glm::i16vec2 vel(glm::rotate(glm::vec2(0, -1), (float)getDir()*glm::half_pi<float>()));

          if (world[pos+vel].get() != nullptr && world[pos+vel]->getType() == Tile::IceWall)
          {
            world[pos+vel]->setType(Tile::None);
          }
          break;
        } case Frog:
          functionCounter = 0;
          break;
      }
      return true;
    }

    bool stepBackward(glm::i16vec2 pos, const World& editorWorld, World& programWorld)
    {
      /*if (programCounter == 0)
      {
        programCounter = -1;
      }*/

      if (functionCounter == 0)
      {
        functionCounter = -1;
      }

      Instruction instruction;
      if (functionCounter == -1)
      {
        if (programCounter == 0)
        {
          return false;
        }
        instruction = program[programCounter--];
      } else
      {
        instruction = function[functionCounter--];
      }

      std::cout << "running instruction " << (int)instruction << '\n';
      switch (instruction)
      {
        case Left:
          sprite.rotate(sf::degrees(90));
          break;
        case Up: {
          glm::i16vec2 vel(-glm::rotate(glm::vec2(0, -1), (float)getDir()*glm::half_pi<float>()));

          programWorld[pos].swap(programWorld[pos+vel]);
          pos += vel;

          if (programWorld[pos-vel-vel].get() != nullptr && programWorld[pos-vel-vel]->getType() == Tile::Crate)
          {
            programWorld[pos-vel].swap(programWorld[pos-vel-vel]);
          }
          break;
        } case Right:
          sprite.rotate(sf::degrees(-90));
          break;
        case Laser: {
          glm::i16vec2 vel(glm::rotate(glm::vec2(0, -1), (float)getDir()*glm::half_pi<float>()));

          if (editorWorld.contains(pos+vel) && editorWorld.find(pos+vel)->second.get() != nullptr && editorWorld.find(pos+vel)->second->getType() == Tile::IceWall)
          {
            programWorld[pos+vel]->setType(Tile::IceWall);
          }
          break;
        } case Frog:
          functionCounter = 0;
          break;
      }
      return true;
    }

    void drawProgram(sf::RenderWindow& SCREEN, const sf::IntRect& bounds)
    {
      sf::RectangleShape instructionSpr(sf::Vector2f(bounds.size.x*0.5f, bounds.size.x*0.5f));
      instructionSpr.setTexture(&Tile::texture);
      instructionSpr.setPosition(sf::Vector2f(0, bounds.position.y));
      instructionSpr.setFillColor(sf::Color((color >= Tile::Pink)*255, (color == Tile::Green)*255, (color == Tile::Blue || color == Tile::Pink)*255));
      instructionSpr.setOutlineColor(instructionSpr.getFillColor());

      for (std::size_t i = programOffset; i < program.size() && instructionSpr.getPosition().y+Tile::spriteSize*instructionSpr.getScale().y < bounds.position.y+bounds.size.y; i++)
      {
        if (programCounter == i)
        {
          instructionSpr.setOutlineThickness(-3);
        }
        instructionSpr.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize * (25+(int)program[i]/2), Tile::spriteSize*((int)program[i]%2)), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
        SCREEN.draw(instructionSpr);
        if (programCounter == i)
        {
          instructionSpr.setOutlineThickness(0);
        }
        instructionSpr.move(sf::Vector2f(0, Tile::spriteSize*instructionSpr.getScale().y));
      }

      instructionSpr.setPosition(sf::Vector2f(Tile::spriteSize*instructionSpr.getScale().x, bounds.position.y));
      for (std::size_t i = functionOffset; i < function.size() && instructionSpr.getPosition().y+Tile::spriteSize*instructionSpr.getScale().y < bounds.position.y+bounds.size.y; i++)
      {
        if (functionCounter == i)
        {
          instructionSpr.setOutlineThickness(-3);
        }
        instructionSpr.setTextureRect(sf::IntRect(sf::Vector2i(Tile::spriteSize * (25+(int)function[i]/2), Tile::spriteSize*((int)function[i]%2)), sf::Vector2i(Tile::spriteSize, Tile::spriteSize)));
        SCREEN.draw(instructionSpr);
        if (functionCounter == i)
        {
          instructionSpr.setOutlineThickness(0);
        }
        instructionSpr.move(sf::Vector2f(0, Tile::spriteSize*instructionSpr.getScale().y));
      }
    }

  private:

    std::vector<Instruction> program;

    std::vector<Instruction> function;

    std::size_t programOffset = 0;
    std::size_t functionOffset = 0;

    std::size_t programCounter = 0;
    std::size_t functionCounter = -1;
};
