#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <functional>
#include <random>

using namespace geode::prelude;

// ─────────────────────────────────────────────────────────────────────────────
// LevelBot  —  Genetic Algorithm that learns to complete GD levels
//
// No API key required. Runs entirely on-device.
//
// How it works:
//   • Each "genome" is a timed sequence of jump actions (click / no-click).
//   • A population of genomes plays the level simultaneously (via physics
//     stepping without rendering).
//   • After each generation the best genomes are selected, crossed, and
//     mutated to produce the next generation.
//   • Fitness = how far the player travelled before dying.
// ─────────────────────────────────────────────────────────────────────────────

struct Genome {
    std::vector<bool> actions; // one entry per physics frame: true = click
    float             fitness; // distance reached (in GD units)
    int               framesDied; // frame index of death (or end)
};

class LevelBot {
public:
    enum class State { Idle, Training, Paused, Completed };

    static LevelBot& get();

    // Start training on the currently loaded level.
    void startTraining();
    void pauseTraining();
    void stopTraining();

    // Replay the best genome found so far.
    void replayBest();

    // Accessors
    State       getState()       const { return m_state; }
    int         getGeneration()  const { return m_generation; }
    float       getBestFitness() const { return m_bestFitness; }
    int         getPopSize()     const { return m_popSize; }
    bool        isSolved()       const { return m_solved; }

    // Called by the GD hooks each physics frame while training is running.
    // Returns the click action the bot wants to perform this frame.
    bool onPhysicsFrame(float playerX, float playerY, bool isDead);

    // Called when a genome's run ends (death or level complete).
    void onRunEnd(float distanceReached, bool levelComplete);

private:
    LevelBot();

    void initPopulation();
    void nextGeneration();
    Genome crossover(const Genome& a, const Genome& b);
    void   mutate(Genome& g);

    std::vector<Genome> m_population;
    Genome              m_bestGenome;
    float               m_bestFitness  = 0.f;
    int                 m_generation   = 0;
    int                 m_currentIdx   = 0; // which genome is running
    int                 m_frame        = 0; // current frame in this run
    int                 m_popSize      = 50;
    bool                m_solved       = false;
    bool                m_replayMode   = false;
    State               m_state        = State::Idle;

    std::mt19937        m_rng;

    // Config (from mod settings)
    void loadConfig();
    int  m_maxFrames   = 3600; // 60 s at 60 fps
    float m_mutRate    = 0.05f;
    float m_eliteRatio = 0.2f;
};
