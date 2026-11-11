#include "flappy.h"

void advancePipes(Game *game, float dt) {
    /* Move all left edges first; recycling uses the positions of every slot. */
    float distance = PIPE_SPEED * dt;
    for (int i = 0; i < PIPE_COUNT; ++i) {
        game->m_pipes[i].m_x -= distance;
    }

    /* Include exact edge contact: a pipe is recyclable once its right edge <= 0. */
    for (int i = 0; i < PIPE_COUNT; ++i) {
        if (game->m_pipes[i].m_x + PIPE_WIDTH <= 0.0f) {
            recyclePipe(game, i);
        }
    }
}

bool birdHitsObstacle(const Game *game) {
    Box bird = gameBirdBox(game);
    /* Touching the ceiling or floor is safe; only crossing the bounds collides. */
    if (bird.m_y < 0.0f || bird.m_y + bird.m_height > FLOOR_Y) {
        return true;
    }

    for (int i = 0; i < PIPE_COUNT; ++i) {
        const Pipe *pipe = &game->m_pipes[i];
        /* boxesOverlap uses strict inequalities, so edge-only contact is safe. */
        Box upper = {pipe->m_x, 0.0f, PIPE_WIDTH, pipe->m_gap_y};
        Box lower = {pipe->m_x, pipe->m_gap_y + PIPE_GAP_HEIGHT, PIPE_WIDTH,
                     FLOOR_Y - pipe->m_gap_y - PIPE_GAP_HEIGHT};
        if (boxesOverlap(bird, upper) || boxesOverlap(bird, lower)) {
            return true;
        }
    }
    return false;
}

int scorePassedPipes(Game *game) {
    int newlyScored = 0;
    for (int i = 0; i < PIPE_COUNT; ++i) {
        Pipe *pipe = &game->m_pipes[i];
        /* Strictly clear the bird before scoring; recycling resets this flag. */
        if (!pipe->m_scored && pipe->m_x + PIPE_WIDTH < BIRD_X) {
            pipe->m_scored = true;
            ++newlyScored;
        }
    }
    game->m_score += newlyScored;
    return newlyScored;
}

unsigned int nextRandom(unsigned int seed) {
    static unsigned int state = RANDOM_INITIAL_SEED;
    if (seed > 0) {
        state = seed % RANDOM_MODULUS;
    }
    state = (RANDOM_MULTIPLIER * state + RANDOM_INCREMENT) % RANDOM_MODULUS;
    return state;
}
