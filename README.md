# Micro and Macro AI

### Introduction
The concept of a micro- and macro-AI is not new. It was first used in *Left 4 Dead* as a dynamic system for pacing and difficulty . The macro AI, also known as the Director AI, spawns enemies depending on several factors, such as: player health, the player’s current ammo, the player’s skill, location and current situation.
The micro AI deals with the separate NPC’s spawned by the macro AI. Each type of NPC often has its own micro AI, ranging from a zombie simply chasing the player, to an enemy trying to outflank a player.
In this research project, I will be focusing on the implementation of, and more extensively, the synergy between, a micro and macro AI in a small game. 

### The Game
I shall make a very simplified version of the game *Alien: Isolation*. The player will wander around the map, and he will flee when he sees the Alien.
The Alien will systematically search the map, based on the input it receives from the macro AI.
This will be a priority system according to its situation: if the Alien sees the player, it will hunt the player. If the Alien is nowhere near the player, the macro AI will systematically direct it towards the player.
The macro AI will also make the Alien go away from the player according to several factors, such as how close the Alien is to the player, and whether the player can run away.

### The Framework
For this research project, I decided not to use Unity or Unreal Engine, but the framework provided to us by Professors Vandaele and Geens, with some slight adjustments, since their framework only provided the necessities for the course. As such, some small features were added, such as:
* The ability to rotate navigation colliders 90 degrees in the navigation mesh.

![Rotate Navigation Colliders](https://github.com/Rhidian12/GPP_Framework/blob/master/RotateNavigationColliders.png)

* Some collision detection functionality, such as ray-casting.

```cpp
// CollisionFunctions.h
namespace Collisions
{
  struct HitInfo
  {
    float lambda;
    Elite::Vector2 intersectPoint;
    Elite::Vector2 normal;
  };

  bool IsPointInRect(const Elite::Vector2& p, const Elite::Rect& r);
  bool IsOverlapping(const Elite::Vector2& a, const Elite::Vector2& b, const Elite::Rect& r, HitInfo& hitInfo);
  bool Raycast(const Elite::Vector2* vertices, const size_t nrVertices, const Elite::Vector2& rayP1, const Elite::Vector2& rayP2, HitInfo& hitInfo);
  bool IntersectLineSegments(const Elite::Vector2& p1, const Elite::Vector2& p2, const Elite::Vector2& q1, const Elite::Vector2& q2, float& outLambda1, float& outLambda2, float epsilon = 1e-6);
  bool IsPointOnLineSegment(const Elite::Vector2& p, const Elite::Vector2& a, const Elite::Vector2& b);
}
```

* An **InvertedBehaviorConditional** node for the behaviour tree. This is easier and cleaner than having to implement **IsXConditionMet** and **IsXConditionNotMet** behaviours.

```cpp
// EBehaviorTree.h
class InvertedBehaviorConditional : public IBehavior
{
public:
    explicit InvertedBehaviorConditional(std::function<bool(Blackboard*)> fp) : m_fpConditional(fp) {}
    virtual BehaviorState Execute(Blackboard* pBlackBoard) override;

private:
    std::function<bool(Blackboard*)> m_fpConditional = nullptr;
};

// EBehaviorTree.cpp
BehaviorState InvertedBehaviorConditional::Execute(Blackboard* pBlackBoard)
{
  if (m_fpConditional == nullptr)
    return Failure;

  switch (!m_fpConditional(pBlackBoard))
  {
    case true:
        return m_CurrentState = Success;
    case false:
        return m_CurrentState = Failure;
  }
  return m_CurrentState = Failure;
}
```

### The Player Micro AI
The player has a limited sensory input, in this case defined by his field of vision. For the purpose of this research project, the player has a limited Finite State Machine:

![Player FSM](https://github.com/Rhidian12/GPP_Framework/blob/master/PlayerFSM.png)

In this FSM (Finite State Machine), the player will start with the Wander behaviour. This is a basic micro AI to demonstrate the workings of the Alien AI and the macro AI.
The player has no health bar, since any hit from the Alien will kill it. The goal of the player is to find all pickups without dying.
Since this research project does not focus on the implementation of this micro AI, the field of view is not bug-free.

```cpp
// App_MicroMacroAI.cpp
const std::vector<Elite::Vector2> App_MicroMacroAI::GetPickupsInPlayerFOV() const
{
  std::vector<Elite::Vector2> pickUpsSeen{};

  const Elite::Vector2 agentPosition{ m_pAgent->GetPosition() };
  const float pickupSize{ 5.f };
  for (const auto& ray : m_PlayerFOVRaycasts)
  {
    for (const auto& pickup : m_Pickups)
    {
      for (const auto& navMeshCollider : m_vNavigationColliders)
      {
        const Elite::Rect colliderHitbox{ navMeshCollider->GetPosition(),navMeshCollider->GetWidth(),navMeshCollider->GetHeight() };
        Collisions::HitInfo hitInfo{};
        if (!Collisions::IsOverlapping(agentPosition, pickup, colliderHitbox, hitInfo))
          if (Elite::IsSegmentIntersectingWithCircle(agentPosition, ray, pickup, pickupSize))
            pickUpsSeen.push_back(pickup);
      }
    }
  }
  return pickUpsSeen;
}
```

Sometimes, the field of view is not able to adjust itself to the colliders in the navigation mesh, resulting in the player being able to see through walls at certain locations. </br>
The choice between a finite state machine and behaviour tree was considered, but eventually a finite state machine was chosen for its simplicity, since the player has a limited number of actions it can take. A behaviour tree, however, would have made a priority queue more accessible. While adding such a system to the finite state machine is possible, it is beyond the scope of this research project; consequently, Booleans were used to ensure a rudimentary priority system. 

```cpp
// Behaviours.cpp
bool HaveNotAllCheckpointsBeenVisited::ToTransition(Blackboard* pBlackboard) const
{
  std::vector<Checkpoint>* pCheckpoints{};
  bool wasPickupSeen{};
  if (!pBlackboard->GetData("checkpoints", pCheckpoints) || !pBlackboard->GetData("wasPickupSeen", wasPickupSeen))
    return false;

  if (wasPickupSeen)
    return false;

  for (const auto& checkpoint : *pCheckpoints)
  {
    if (!checkpoint.hasBeenVisited)
      return true;
  }
  return false;
}
```
Before the player returns to the search pattern, a Boolean, which is set to true if a pickup is seen, is checked, ensuring that the player will not go to a checkpoint if he sees a pickup.
This Boolean is set to false when the pickup has been picked up by the player.

### The Alien Micro AI
Like the player, the Alien has limited sensory input. At the start of the game, it will only be able to see the player if it enters its field of view.</br>
When the player has picked up 50% of the pickups, the Alien will gain a new steering behaviour: pursuit. Instead of simply following the player, the Alien will try and cut off the player.

In what way, however, would this evolving behaviour be simulated? At first, a behaviour tree was considered with several nodes which were “locked” behind a conditional node until the macro AI “unlocks” the condition for the node to return true. This is a possible implementation, but a rather basic one. The behaviour tree would be expanding rapidly in a more complex game, with multiple sequence nodes containing multiple conditional and action nodes.

![Alien BHT](https://github.com/Rhidian12/GPP_Framework/blob/master/AlienBHT1.png)

This is a small example of what a behaviour tree would look like in this case. These are 25 nodes for 2 steering behaviours, with no search pattern or a job system included. The behaviour tree would rapidly expand, and it would become? more difficult to debug the further the project advances.

A finite state machine was also considered, but this appears to be the most flawed micro AI of all possible ideas. Although a finite state machine would be possible for the scope of this research project, it would be too complex to maintain for a more complex micro AI.

![Alien FSM](https://github.com/Rhidian12/GPP_Framework/blob/master/AlienFSM.png)

This is the same behaviour tree as listed above, with a wander behaviour and a search behaviour added, in the form of a finite state machine. This has already become a far too complex diagram to understand.

In *Left 4 Dead*, the macro AI simply spawns enemies, but it does not control or alter them in any fashion, except for de-spawning enemies that are too distant from the player. In Alien: Isolation, the macro AI allows the micro AI to evolve, but it also helps it to locate the player. Although the micro AI must find and kill the player itself, the macro AI will periodically direct the micro AI to the general location of the player, thereby ensuring that the micro AI does not simply get lost in the map.</br>
The macro AI also helps the player survive the micro AI’s attacks. When it detects that the micro AI is going to corner the player, without any hope of survival, the macro AI will temporarily lead the micro AI away from the player, or make the micro AI enter its cooldown phase. 

Within the scope of this research project, the micro AI will have a limited behaviour tree to find and kill the player. It will detect the player using multiple fields of view. 

![Alien FOV](https://github.com/Rhidian12/GPP_Framework/blob/master/AlienFOV.png)

The micro AI will use the red field of view to see the player, but the player is not instantly detected. If the player stays in this field of view for 2 seconds, the micro AI will move toward the player and try to kill the player.</br>
The green field of view is shorter than the red, but if the player walks into this field of view, he is instantly detected by the micro AI, and he will be assaulted.
The final purple field of view is meant to ensure that the player cannot simply walk by the micro AI, and he will be instantly detected if he enters this field of vision.

Because of the research project’s limited scope, the micro AI’s behaviour tree is restricted, and it will feature some conditional nodes which are enabled by the macro AI.

The interesting question is how the micro AI’s behaviour tree is structured. As stated earlier in this document, although a behaviour tree featuring conditional nodes related to the macro AI is entirely possible, there must be other implementations.

![Alien BHT](https://github.com/Rhidian12/GPP_Framework/blob/master/AlienBHT2.png)

A possible implementation of this schematic would be to make a job system. The micro AI has a set of jobs it executes infinitely, and the macro AI can assign a priority job, a task which must be completed as soon as possible. When the macro AI assigns a priority job is addressed later.

```cpp
// Job.h
#include "stdafx.h"
#include <functional>
class Blackboard;
enum class JobState
{
  FAILURE = 0,
  RUNNING = 1,
  COMPLETE = 2
};
enum class JobPriority
{
  PRIORITY = 0,
  NORMAL = 1
};
enum class JobType
{
  INVESTIGATE = 0,
  COOLDOWN = 1,
};
class Job
{
public:
  explicit Job(std::function<JobState(Elite::Blackboard* pAlienBlackboard)> pJobFunction, const JobPriority& jobPriority, const JobType& jobType);
  ~Job() = default;

  const JobState ExecuteJob(Elite::Blackboard* pBlackboard);
  const JobPriority GetJobPriority() const;
  const JobType GetJobType() const;

private:
  std::function<JobState(Elite::Blackboard* pAlienBlackboard)> m_pJobFunction;
  JobPriority m_JobPriority;
  JobType m_JobType;
};
```

For this research project, only two jobs were created, but many more could be added. A job has a priority, a type, and its current state. The micro AI will then in its behaviour tree execute this job until it returns a JobState COMPLETE. At that point, the micro AI generates a new job which it can then perform.</br>
At a first glance, this way of creating jobs seemed flawless, but on closer inspection, the technique becomes increasingly hard to maintain, as the project expands in scope.

The first reason for this is that the Alien keeps its jobs as a deque of job pointers.

```cpp
// Alien.h
#include "MicroAIAgent.h"
#include <deque>
class Job;
class Alien final : public MicroAIAgent
{
public:
  Alien(const Elite::Vector2& position);
  virtual ~Alien();

  virtual void Update(float dt) override;
  virtual void UpdateDecisionMaking(float dt) override;
  virtual void Render(float dt) override;

  void AddJob(Job* pJob);

  std::deque<Job*>& GetJobs();

private:
  std::deque<Job*> m_pJobs;
};
```
The choice between a separate queue for each type of job priority, and a single deque, was considered, but eventually a deque was preferred, for its simplicity. The use of a deque, however, presents multiple disadvantages that do not become apparent in the scope of this research project.</br>
In the implementation that was made, only one priority job can be in the jobs deque. When a job is added, it is either pushed to the front or the back of the deque, depending on the job’s priority. Priority jobs get pushed to the front, while normal jobs get pushed to the back.

```cpp
// Alien.cpp
void Alien::AddJob(Job* pJob)
{
  if (pJob->GetJobPriority() == JobPriority::NORMAL)
    m_pJobs.push_back(pJob);
  else
    m_pJobs.push_front(pJob);
}
```
If multiple priority jobs were to be added, we would go from a first in, first out system to a sort of first in, last out system. Although all the priority jobs would still be at the front of the deque, they would not be performed in the order that they were added.

![Alien Job Deque](https://github.com/Rhidian12/GPP_Framework/blob/master/AlienJobDeque.png)

Priority jobs that are added later will force priority jobs that were added earlier to the back, creating, as mentioned above, a first in, last out order of performance.
This could be fixed in a multitude of ways:

* Multiple levels of priority, where priority jobs are always more important than normal jobs but are less/more important than other jobs, depending on their level of priority.
* Using different lists for each level of priority.
* Creating a linked list where you could easily create an insertion system, although like in most containers, inserting is a relatively slow operation in comparison to pushing something to the front/back of the queue.

Another issue is how the AI performs a job.
```cpp
// Job.cpp
const JobState Job::ExecuteJob(Elite::Blackboard* pBlackboard)
{
  return m_pJobFunction(pBlackboard);
}

// AlienBehaviours.cpp
Elite::BehaviorState ExecuteFirstJob(Elite::Blackboard* pBlackboard)
{
  std::deque<Job*>* pJobs{};
  Alien* pAlien{};
  MicroAIAgent* pPlayer{};
  if (!pBlackboard->GetData("jobs", pJobs) || !pBlackboard->GetData("alien", pAlien) || !pBlackboard->GetData("agent", pPlayer))
    return Elite::BehaviorState::Failure;

  const JobState currentJobState{ (*pJobs)[0]->ExecuteJob(pBlackboard) };

  switch (currentJobState)
  {
  case JobState::COMPLETE:
    switch ((*pJobs)[0]->GetJobType())
    {
      SAFE_DELETE((*pJobs)[0]);
      pJobs->pop_front();

    case JobType::INVESTIGATE:
    case JobType::COOLDOWN:
      if (pJobs->empty())
      {
        const Elite::Vector2 randomPosition{ CalculateNewInvestigationArea(pBlackboard) };
        pBlackboard->ChangeData("investigationTarget", randomPosition);
        pAlien->AddJob(new Job{ InvestigateArea,JobPriority::NORMAL,JobType::INVESTIGATE });
      }
      return Elite::BehaviorState::Success;
      break;
    default:
      break;
    }
    break;
  case JobState::RUNNING:
    return Elite::BehaviorState::Success;
    break;
  case JobState::FAILURE:
#ifdef _DEBUG || DEBUG
    assert(currentJobState != JobState::FAILURE);
#endif
    return Elite::BehaviorState::Failure;
    break;
  default:
      break;
  }
  return Elite::BehaviorState::Failure;
}
```
For JobStates RUNNING and FAILURE there is no extra code, but in the case of a COMPLETE JobState the first job gets deleted and replaced by a new one. In the present limited implementation of micro and macro AI, this is fine, but when the game’s complexity is increased, this way of deleting and creating new jobs becomes impossible to maintain. </br>For every type of job several new helper functions would have to be created, in order to create a new job if required. In our case, the Cooldown job can only be assigned by the macro AI, so every type of job can simply create a new Investigate job. </br>If jobs were to transition, however, into different jobs depending on the last job executed, we would have to write a lot of duplicate code for a lot of different job types.</br>
This could be resolved by adding a finite state machine for the job system.

![Alien Job FSM](https://github.com/Rhidian12/GPP_Framework/blob/master/AlienJobFSM.png)

This would be a better and cleaner implementation of performing and adding new jobs in the behaviour tree.</br>
A finite state machine is not absolutely necessary; an event system could also be used to recreate a similar system. An event system might be the cleanest solution, and it would not be very hard to implement, as different queues or a different type of container could be used.

### The Macro AI
The macro AI controls the flow of the game. In *Left 4 Dead* it handles enemy spawns and health pickups. In *Alien: Isolation* it directly influences the Alien’s behaviour tree by allowing it to “learn” how the player escapes the micro AI.</br>
How does the macro AI accomplish this? It is a complex algorithm that checks certain game flags, player skill, player inventory, player health, … everything in the world relating to the players and the level itself.</br>
In this research project’s implementation of the macro AI, it is very limited, since the world and the game itself are not complex.
However, in a larger project, a possible implementation of the macro-AI could look like this:
```cpp
// not implemented
std::unordered_map<std::string, std::function<bool(Elite::Blackboard*)>> m_Events{};

for (const auto& event : m_Events)
  if (event.second(pBlackboard))
      pBlackboard->ChangeData(event.first, true);
```
This implementation has the advantage of being easy to use. Flags could be placed inside the Alien’s blackboard and set to true as soon as a condition is met by the macro AI. This would, in turn, cause the micro AI’s behaviour tree to change, or set a spawn flag to true somewhere in the world.</br>
This could also be easily expanded upon, and each function wrapper could also change things in the blackboard/world itself.

A drawback to this system would be that the map is unordered. Thus, any sort of priority has been lost, in exchange for performance, which is a choice that must be made in the context of the project. Although this specific implementation does not have any sort of priority system, the macro AI does not particularly need a priority system. </br>An event could be added to the map to check after a certain point in the level has been reached.</br>
For example, if the players defeated the first boss in a level, an event that should be checked regularly could be added to the unordered map, while another event could be removed.</br>
An ordered map would also be possible, however it would be slower than an unordered map and, depending on the type of key, a hash function and comparator must be made.

### Synergy Between Micro and Macro AI
How do *Left 4 Dead*, *Alien: Isolation*, and other games with a micro and macro AI, such as RTS-games, handle this problem? They all have a macro AI, and *Alien: Isolation’s* micro AI features a behaviour tree with over 300 nodes .
The macro AI works differently for *Left 4 Dead* and *Alien: Isolation*. In the case of *Left 4 Dead*, the macro AI decides on mob spawns. The better the players are, the harder the level is. The macro AI also tries to create an ebb-and-flow dynamic into the game, by allowing players to recover after a prolonged fight, or meeting heavy resistance when rapidly advancing through the level.</br>
The macro AI achieves this goal by dynamically spawning enemies based on player skill and their current situation. The difficulty of the game will slightly decrease if the players are less skilled or when they are in a precarious situation. On the flip side, the game will get harder if the players are very skilled or when they are in an overly relaxed situation. The macro AI will then spawn more enemies more quickly. The macro AI has very little control over item spawns in Left 4 Dead; the only item it controls are health pickups. This is meant to give the macro AI more control over the flow of the game, since it can determine how many health pickups and what kind of health pickups the player needs. All of this should contribute to the flow of the game.</br>
The macro AI in *Alien: Isolation* also controls the flow of the game, but unlike the macro AI in *Left 4 Dead*, it directly influences the micro AI. This is meant to create the illusion that the micro AI is learning from the player’s playstyle. If the player can avoid the micro AI several times by hiding in a closet, the macro AI will unlock that part of the micro AI’s behaviour tree that is responsible for checking closets. Consequently, the player’s method of eluding the micro AI cannot work indefinitely, forcing the player to learn and adapt.</br>
The micro and macro AI are also more connected in *Alien: Isolation* than they are in *Left 4 Dead*. 

### Conclusion
The macro AI is solely used to control the flow of the game. It decides when enemies need to be spawned, when the micro AI unlocks its new behaviour, how much health pickups the player can find, and so on. It does this by dynamically checking the world and the players.</br>
A macro AI never directly manages a micro AI. The micro AI has its own tasks to perform and it is responsible for those tasks. The macro-AI can nonetheless assign new jobs to the micro AI or enable it to use one of its more advanced behaviours.</br>
The micro AI is mostly unaware of the macro AI. It has its own decision-making structure and it is dependent on that.

The synergy between micro and macro AI is at peak display in both *Left 4 Dead* and *Alien: Isolation*, even though there are very few similarities between the games.
Both games expertly control the flow of the game to create the most enjoyable experience possible, by ensuring that the game is hard enough, but not brutally impossible, which is the final and most important goal of the macro AI.



### Sources

1. X. s.d..The Director. Left4DeadFandom. Retrieved from https://left4dead.fandom.com/wiki/The_Director
2. Thompson, T.  (2017, April 24). The Perfect Organism – The AI of Alien: Isolation. Medium. Retrieved from https://becominghuman.ai/the-perfect-organism-d350c05d8960
3. X. s.d.. The Director. Left4Dead Fandom. Retrieved from https://left4dead.fandom.com/wiki/The_Director#Director_Phases
4. AI and Games. (2020, May 20). Revisiting the AI of Alien: Isolation | AI and Games [Video]. Youtube. https://www.youtube.com/watch?v=P7d5lF6U0eQ
5. AI and Games. (2014, December 1). The Director AI of Left 4 Dead | AI and Games [Video]. Youtube. https://www.youtube.com/watch?v=WbHMxo11HcU&ab_channel=AIandGames
