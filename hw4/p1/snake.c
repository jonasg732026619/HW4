#include "snake.h"
#include <stdlib.h>

static bool isValidDirection(Direction direction) {
    return direction >= DIR_UP && direction < DIRECTION_COUNT;
}

static bool isOpposite(Direction left, Direction right) {
    return (left == DIR_UP && right == DIR_DOWN) ||
           (left == DIR_DOWN && right == DIR_UP) ||
           (left == DIR_LEFT && right == DIR_RIGHT) ||
           (left == DIR_RIGHT && right == DIR_LEFT);
}

static Direction effectiveDirection(const Snake *snake, Direction requested) {
    if (!isValidDirection(requested)) {
        return snake->m_direction;
    }
    if (snake->m_length > 1 && isOpposite(snake->m_direction, requested)) {
        return snake->m_direction;
    }
    return requested;
}

static Coord movedPosition(Coord position, Direction direction) {
    switch (direction) {
        case DIR_UP:
            --position.m_row;
            break;
        case DIR_RIGHT:
            ++position.m_col;
            break;
        case DIR_DOWN:
            ++position.m_row;
            break;
        case DIR_LEFT:
            --position.m_col;
            break;
        case DIRECTION_COUNT:
            break;
    }
    return position;
}

bool snakeInit(Snake *snake, int rows, int cols, size_t length) {
    if (snake == NULL) {
        return false;
    }
    *snake = (Snake){0};
    if (rows < MIN_BOARD_SIZE || cols < MIN_BOARD_SIZE || length == 0 ||
        length > (size_t)(cols - 2 * BOARD_WALL_THICKNESS)) {
        return false;
    }

    int headCol = cols / 2;
    if (length > (size_t)headCol) {
        headCol = (int)length;
    }
    int headRow = rows / 2;
    SnakeNode *tail = NULL;
    SnakeNode *last = NULL;

    for (size_t i = 0; i < length; ++i) {
        SnakeNode *node = malloc(sizeof(*node));
        if (node == NULL) {
            /* The partial list is linear until all nodes have been allocated. */
            while (tail != NULL) {
                SnakeNode *next = tail->m_next;
                free(tail);
                tail = next;
            }
            *snake = (Snake){0};
            return false;
        }
        size_t offset = length - 1 - i;
        node->m_pos = (Coord){headRow, headCol - (int)offset};
        node->m_next = NULL;
        if (tail == NULL) {
            tail = node;
        } else {
            last->m_next = node;
        }
        last = node;
    }

    /* Close the ring: tail -> body -> head -> tail. */
    last->m_next = tail;
    snake->m_head = last;
    snake->m_length = length;
    snake->m_direction = DIR_RIGHT;
    snake->m_rows = rows;
    snake->m_cols = cols;
    return true;
}

Coord snakeNextPosition(const Snake *snake, Direction requested) {
    Direction direction = effectiveDirection(snake, requested);
    return movedPosition(snake->m_head->m_pos, direction);
}

bool snakeContains(const Snake *snake, Coord pos) {
    if (snake == NULL || snake->m_head == NULL || snake->m_length == 0) {
        return false;
    }

    const SnakeNode *node = snake->m_head;
    for (size_t i = 0; i < snake->m_length; ++i) {
        if (node == NULL) {
            return false;
        }
        if (node->m_pos.m_row == pos.m_row && node->m_pos.m_col == pos.m_col) {
            return true;
        }
        node = node->m_next;
    }
    return false;
}

StepResult snakeStep(Snake *snake, Direction requested, bool grow) {
    Direction direction = effectiveDirection(snake, requested);
    Coord nextPosition = movedPosition(snake->m_head->m_pos, direction);

    if (nextPosition.m_row < BOARD_WALL_THICKNESS ||
        nextPosition.m_row >= snake->m_rows - BOARD_WALL_THICKNESS ||
        nextPosition.m_col < BOARD_WALL_THICKNESS ||
        nextPosition.m_col >= snake->m_cols - BOARD_WALL_THICKNESS) {
        return STEP_WALL;
    }

    SnakeNode *tail = snake->m_head->m_next;
    SnakeNode *node = snake->m_head;
    for (size_t i = 0; i < snake->m_length; ++i) {
        if (node->m_pos.m_row == nextPosition.m_row &&
            node->m_pos.m_col == nextPosition.m_col && (grow || node != tail)) {
            return STEP_SELF;
        }
        node = node->m_next;
    }

    if (grow) {
        /* Insert a new head immediately before the old tail in ring order. */
        SnakeNode *newHead = malloc(sizeof(*newHead));
        if (newHead == NULL) {
            return STEP_NO_MEMORY;
        }
        newHead->m_pos = nextPosition;
        newHead->m_next = tail;
        snake->m_head->m_next = newHead;
        snake->m_head = newHead;
        ++snake->m_length;
    } else {
        /* The old tail becomes the head; existing ring links stay unchanged. */
        tail->m_pos = nextPosition;
        snake->m_head = tail;
    }
    snake->m_direction = direction;
    return STEP_OK;
}

void snakeDestroy(Snake *snake) {
    if (snake == NULL) {
        return;
    }
    SnakeNode *node = snake->m_head;
    for (size_t i = 0; i < snake->m_length && node != NULL; ++i) {
        SnakeNode *next = node->m_next;
        free(node);
        node = next;
    }
    *snake = (Snake){0};
}

unsigned int nextRandom(unsigned int seed) {
    static unsigned int state = RANDOM_INITIAL_SEED;
    if (seed > 0) {
        state = seed % RANDOM_MODULUS;
    }
    state = (RANDOM_MULTIPLIER * state + RANDOM_INCREMENT) % RANDOM_MODULUS;
    return state;
}
