#include "LevelBot.h"
#include <Geode/loader/Mod.hpp>
#include <algorithm>
#include <numeric>

using namespace geode::prelude;

// ─────────────────────────────────────────────────────────────────────────────
LevelBot& LevelBot::get() {
    static LevelBot inst;
    return inst;
}

LevelBot::LevelBot() : m_rng(std::random_device{}()) {
    loadConfig();
}

void LevelBot::loadConfig() {
    m_popSize  = Mod::get()->getSettingValue<int>("population-size");
    // bot-speed is used by the hook to throttle; maxFrames covers ~60 s of play
    m_maxFrames = 60 * 60; // 3600 frames @ 60fps
}

// ─────────────────────────────────────────────────────────────────────────────
// Training lifecycle
// ─────────────────────────────────────────────────────────────────────────────
void LevelBot::startTraining() {
    loadConfig();
    m_state      = State::Training;
    m_generation = 0;
    m_bestFitness= 0.f;
    m_solved     = false;
    m_replayMode = false;
    initPopulation();
    m_currentIdx = 0;
    m_frame      = 0;
    log::info("[LevelBot] Training started. Pop={}, maxFrames={}", m_popSize, m_maxFrames);
}

void LevelBot::pauseTraining() {
    if (m_state == State::Training) m_state = State::Paused;
    else if (m_state == State::Paused) m_state = State::Training;
}

void LevelBot::stopTraining() {
    m_state = State::Idle;
    log::info("[LevelBot] Training stopped at gen {}. Best fitness={:.1f}", m_generation, m_bestFitness);
}

void LevelBot::replayBest() {
    if (m_bestFitness <= 0.f) return;
    m_replayMode = true;
    m_frame      = 0;
    log::info("[LevelBot] Replaying best genome (fitness={:.1f})", m_bestFitness);
}

// ─────────────────────────────────────────────────────────────────────────────
// Population helpers
// ─────────────────────────────────────────────────────────────────────────────
void LevelBot::initPopulation() {
    m_population.resize(m_popSize);
    std::uniform_int_distribution<int> dist(0, 1);
    for (auto& g : m_population) {
        g.actions.resize(m_maxFrames);
        g.fitness = 0.f;
        g.framesDied = m_maxFrames;
        for (auto& a : g.actions)
            a = (dist(m_rng) == 1);
    }
}

Genome LevelBot::crossover(const Genome& a, const Genome& b) {
    Genome child;
    child.actions.resize(m_maxFrames);
    child.fitness = 0.f;
    child.framesDied = m_maxFrames;
    std::uniform_int_distribution<int> cut(0, m_maxFrames - 1);
    int point = cut(m_rng);
    for (int i = 0; i < m_maxFrames; i++)
        child.actions[i] = (i < point) ? a.actions[i] : b.actions[i];
    return child;
}

void LevelBot::mutate(Genome& g) {
    std::uniform_real_distribution<float> prob(0.f, 1.f);
    for (auto& a : g.actions)
        if (prob(m_rng) < m_mutRate)
            a = !a;
}

void LevelBot::nextGeneration() {
    // Sort descending by fitness
    std::sort(m_population.begin(), m_population.end(),
        [](const Genome& a, const Genome& b) { return a.fitness > b.fitness; });

    // Track best
    if (m_population[0].fitness > m_bestFitness) {
        m_bestFitness = m_population[0].fitness;
        m_bestGenome  = m_population[0];
        log::info("[LevelBot] Gen {} — new best fitness={:.1f}", m_generation, m_bestFitness);
    }

    // Elite selection: keep top eliteRatio
    int eliteCount = std::max(2, (int)(m_popSize * m_eliteRatio));
    std::vector<Genome> nextPop(m_population.begin(), m_population.begin() + eliteCount);

    std::uniform_int_distribution<int> parentDist(0, eliteCount - 1);
    while ((int)nextPop.size() < m_popSize) {
        int ia = parentDist(m_rng);
        int ib = parentDist(m_rng);
        if (ia == ib) ib = (ib + 1) % eliteCount;
        Genome child = crossover(m_population[ia], m_population[ib]);
        mutate(child);
        nextPop.push_back(std::move(child));
    }

    m_population = std::move(nextPop);
    m_generation++;
    m_currentIdx = 0;
    m_frame      = 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Per-frame hook (called by GD hooks in main.cpp)
// ─────────────────────────────────────────────────────────────────────────────
bool LevelBot::onPhysicsFrame(float /*playerX*/, float /*playerY*/, bool isDead) {
    if (m_state != State::Training && !m_replayMode) return false;
    if (isDead) return false;

    if (m_replayMode) {
        if (m_frame >= m_maxFrames) return false;
        return m_bestGenome.actions[m_frame++];
    }

    // Training
    if (m_currentIdx >= m_popSize) {
        nextGeneration();
    }
    if (m_frame >= m_maxFrames) {
        onRunEnd(0.f, false); // Timed out
        return false;
    }
    return m_population[m_currentIdx].actions[m_frame++];
}

void LevelBot::onRunEnd(float distanceReached, bool levelComplete) {
    if (m_currentIdx < m_popSize) {
        m_population[m_currentIdx].fitness    = distanceReached;
        m_population[m_currentIdx].framesDied = m_frame;
    }

    if (levelComplete) {
        m_solved = true;
        m_state  = State::Completed;
        if (m_currentIdx < m_popSize)
            m_bestGenome = m_population[m_currentIdx];
        m_bestFitness = distanceReached;
        log::info("[LevelBot] LEVEL SOLVED on gen {}!", m_generation);
        return;
    }

    m_currentIdx++;
    m_frame = 0;
    if (m_currentIdx >= m_popSize)
        nextGeneration();
}
