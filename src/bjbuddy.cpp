// TODO: Implment all deck actions
// - Shuffling
// - Drawing Cards
// - Concept of a hand?
//
// Also need to add general player actions
#pragma once
#include "core.unity.h"

global u32 MAX_HAND_COUNT = 13;

inline static void Hit(deck *Deck, hand *Hand)
{
  NeoAssert(Deck->current - Deck->cards < 52);
  if (Hand->card_count >= 13) return;
  card DrawnCard = *Deck->current++;

  // TODO: Need to handle making the ACE valued at 11 again
  // in the split function.
  if (DrawnCard.type == ACE && (Hand->value + DrawnCard.value) > 21)
  {
    DrawnCard.value = 1;
  }

  Hand->cards[Hand->card_count++] = DrawnCard;
  Hand->value += DrawnCard.value;
}

#if 0
inline static void shuffle(card deck[], mem_index deck_size)
{
  time_t t;
  srand((unsigned) time(&t));
  for (int i = 0; i < deck_size - 2; i++)
  {
    int j = rand() % (deck_size - 2 - i);
    // Swap
    card temp = deck[i];
    deck[i] = deck[j];
    deck[j] = temp;
  }
}
#else
// NOTE: This is Fisher-Yates Algo
// TODO: Need to implement random numbers
inline static void Shuffle(card *Cards, ums CardCount)
{
  u32 RandomIndex = 0;
  for (ums Index = 0; Index <= CardCount - 1; Index++)
  {
    ums SwapIndex = RandomNumberTable[RandomIndex++] % (CardCount - 1);
    // Swap
    card Temp = Cards[Index];
    Cards[Index] = Cards[SwapIndex];
    Cards[SwapIndex] = Temp;
  }
}
#endif

#if 0
#include "neo_jpg.h"
#include "neo_jpg.cpp"
#endif

static inline void NextPhase(enum phase *Phase)
{
    *Phase = (enum phase)((s32)(*Phase) + 1);
}

static inline char *TypeToCStr(s32 CardType)
{
  switch(CardType)
  {
    case ACE: return "ACE";
    case TWO: return "TWO";
    case THREE: return "THREE";
    case FOUR: return "FOUR";
    case FIVE: return "FIVE";
    case SIX: return "SIX";
    case SEVEN: return "SEVEN";
    case EIGHT: return "EIGHT";
    case NINE: return "NINE";
    case TEN: return "TEN";
    case JACK: return "JACK";
    case QUEEN: return "QUEEN";
    case KING: return "KING";
  }

  return "";
}
static inline char *SuitToCStr(s32 CardSuit)
{
  switch(CardSuit)
  {
    case SPADES: return "SPADES";
    case HEARTS: return "HEARTS";
    case DIAMONDS: return "DIAMONDS";
    case CLUBS: return "CLUBS";
  }

  return "";
}

static void ResetRound(app_state *GameState)
{
  GameState->dealer.card_count = 0;
  GameState->dealer.value = 0;

  GameState->player.card_count = 0;
  GameState->player.value = 0;
}

static void UpdateAndRender(thread_context *Thread, app_memory *Memory, engine_input *Input, renderer *Renderer)
{
  NeoAssert(sizeof(app_state) <= Memory->perm_storage_size);
  app_state *GameState = (app_state *)Memory->perm_memory;

  if(!Memory->is_init)
  {
    InitArena(&GameState->core_arena, Memory->perm_storage_size - sizeof(app_state),
              (u8 *)Memory->perm_memory + sizeof(app_state));

    GameState->base_deck = {
      {
        { TWO, SPADES, 2 },
        { THREE, SPADES, 3 },
        { FOUR, SPADES, 4 },
        { FIVE, SPADES, 5 },
        { SIX, SPADES, 6 },
        { SEVEN, SPADES, 7 },
        { EIGHT, SPADES, 8 },
        { NINE, SPADES, 9 },
        { TEN, SPADES, 10 },
        { JACK, SPADES, 10 },
        { QUEEN, SPADES, 10 },
        { KING, SPADES, 10 },
        { ACE, SPADES, 11 },
        { TWO, HEARTS, 2 },
        { THREE, HEARTS, 3 },
        { FOUR, HEARTS, 4 },
        { FIVE, HEARTS, 5 },
        { SIX, HEARTS, 6 },
        { SEVEN, HEARTS, 7 },
        { EIGHT, HEARTS, 8 },
        { NINE, HEARTS, 9 },
        { TEN, HEARTS, 10 },
        { JACK, HEARTS, 10 },
        { QUEEN, HEARTS, 10 },
        { KING, HEARTS, 10 },
        { ACE, HEARTS, 11 },
        { TWO, CLUBS, 2 },
        { THREE, CLUBS, 3 },
        { FOUR, CLUBS, 4 },
        { FIVE, CLUBS, 5 },
        { SIX, CLUBS, 6 },
        { SEVEN, CLUBS, 7 },
        { EIGHT, CLUBS, 8 },
        { NINE, CLUBS, 9 },
        { TEN, CLUBS, 10 },
        { JACK, CLUBS, 10 },
        { QUEEN, CLUBS, 10 },
        { KING, CLUBS, 10 },
        { ACE, CLUBS, 11 },
        { TWO, DIAMONDS, 2 },
        { THREE, DIAMONDS, 3 },
        { FOUR, DIAMONDS, 4 },
        { FIVE, DIAMONDS, 5 },
        { SIX, DIAMONDS, 6 },
        { SEVEN, DIAMONDS, 7 },
        { EIGHT, DIAMONDS, 8 },
        { NINE, DIAMONDS, 9 },
        { TEN, DIAMONDS, 10 },
        { JACK, DIAMONDS, 10 },
        { QUEEN, DIAMONDS, 10 },
        { KING, DIAMONDS, 10 },
        { ACE, DIAMONDS, 11 }
      },
    };

    // Blackjack Setup

    GameState->game_phase = START;
    for (s32 Index = 0; Index < 4; ++Index)
    {
      Shuffle(GameState->base_deck.cards,
              ArrayCount(GameState->base_deck.cards));
    }
    // TODO: This is to 'burn' the first card of the deck but I should
    // make this into an option/setting for the type of game.
    GameState->base_deck.current = &GameState->base_deck.cards[1];

    hand PlayerHand = {0};
    PlayerHand.cards = PushArray(&GameState->core_arena, MAX_HAND_COUNT, card);
    hand DealerHand = {0};
    DealerHand.cards = PushArray(&GameState->core_arena, MAX_HAND_COUNT, card);

    Hit(&GameState->base_deck, &PlayerHand);
    Hit(&GameState->base_deck, &DealerHand);
    Hit(&GameState->base_deck, &PlayerHand);
    Hit(&GameState->base_deck, &DealerHand);

    GameState->dealer = DealerHand;
    GameState->player = PlayerHand;

    NextPhase(&GameState->game_phase);

    Memory->is_init = true;
  }

  if (GameState->game_phase == START)
  {
    hand *DealerHand = &GameState->dealer;
    hand *PlayerHand = &GameState->player;

    Hit(&GameState->base_deck, PlayerHand);
    Hit(&GameState->base_deck, DealerHand);
    Hit(&GameState->base_deck, PlayerHand);
    Hit(&GameState->base_deck, DealerHand);

    NextPhase(&GameState->game_phase);
  }
  if (GameState->game_phase == END)
  {
    ResetRound(GameState);
    GameState->game_phase = START;
  }
  ResetRenderer(Renderer);

  for (ums ControllerIndex = 0;
  ControllerIndex < ArrayCount(Input->controllers);
  ControllerIndex++)
  {
    engine_controller_input *Controller = GetController(Input, ControllerIndex);
    if (Controller->is_analog)
    {
    }
    else
    {
      engine_button_state ActionDown = Controller->action_down;
      engine_button_state ActionUp = Controller->action_up;
      engine_button_state ActionRight = Controller->action_right;
      engine_button_state ActionLeft = Controller->action_left;
      engine_button_state RShoulder = Controller->right_shoulder;

      switch (GameState->game_phase)
      {
        case START:
        {
          if (ActionRight.half_transitions != 0 && ActionRight.is_down)
          {
            char OutStr[256];
            card *Cards = GameState->base_deck.current;
            OutputDebugStringA("Deck (current)\n");
            for (u32 Index = 0; Index < 4; ++Index)
            {
              _snprintf_s(OutStr, sizeof(OutStr), "RANK: %s SUIT: %s\n",
                          TypeToCStr(Cards[Index].type), SuitToCStr(Cards[Index].suit));
              OutputDebugStringA(OutStr);
            }
            OutputDebugStringA("======================\n");
          }
        } break;
        case PLAYER:
        {
          if (ActionLeft.half_transitions != 0 && ActionLeft.is_down)
          {
            Hit(&GameState->base_deck, &GameState->player);

            char OutStr[256];
            _snprintf_s(OutStr, sizeof(OutStr), "Hand Value: %d\n", GameState->player.value);
            OutputDebugStringA(OutStr);

            OutputDebugStringA("======================\n");
          }
          else if (RShoulder.half_transitions != 0 && RShoulder.is_down)
          {
            GameState->game_phase = DEALER;
          }
        } break;
        case DEALER:
        {
          if (ActionLeft.half_transitions != 0 && ActionLeft.is_down)
          {
            Hit(&GameState->base_deck, &GameState->dealer);

            char OutStr[256];
            _snprintf_s(OutStr, sizeof(OutStr), "Hand Value: %d\n", GameState->dealer.value);
            OutputDebugStringA(OutStr);

            OutputDebugStringA("======================\n");
          }
        } break;

        default:
        {
          if (ActionDown.half_transitions != 0 && ActionDown.is_down)
          {
            char OutStr[256];
            card *Cards = GameState->base_deck.cards;
            OutputDebugStringA("Deck (Base) Output:\n");
            for (s32 Index = 0; Index < 8; ++Index)
            {
              _snprintf_s(OutStr, sizeof(OutStr), "RANK: %s SUIT: %s\n",
                          TypeToCStr(Cards[Index].type), SuitToCStr(Cards[Index].suit));
              OutputDebugStringA(OutStr);
            }
            OutputDebugStringA("======================\n");
          }
          else if (ActionUp.half_transitions != 0 && ActionUp.is_down)
          {
            char OutStr[256];
            hand PlayerHand = GameState->player;
            card *Cards = PlayerHand.cards;
            OutputDebugStringA("Player's Hand\n");
            for (u32 Index = 0; Index < PlayerHand.card_count; ++Index)
            {
              _snprintf_s(OutStr, sizeof(OutStr), "RANK: %s SUIT: %s\n",
                          TypeToCStr(Cards[Index].type), SuitToCStr(Cards[Index].suit));
              OutputDebugStringA(OutStr);
            }
            OutputDebugStringA("======================\n");
          }
          else if (ActionRight.half_transitions != 0 && ActionRight.is_down)
          {
            char OutStr[256];
            card *Cards = GameState->base_deck.current;
            OutputDebugStringA("Deck (current)\n");
            for (u32 Index = 0; Index < 4; ++Index)
            {
              _snprintf_s(OutStr, sizeof(OutStr), "RANK: %s SUIT: %s\n",
                          TypeToCStr(Cards[Index].type), SuitToCStr(Cards[Index].suit));
              OutputDebugStringA(OutStr);
            }
            OutputDebugStringA("======================\n");
          }
        }
      }
    }
  }

  // UPDATE
  b32 PlayerBust = GameState->game_phase == PLAYER && GameState->player.value > 21;
  b32 DealerBust = GameState->game_phase == DEALER && GameState->dealer.value > 21;
  if (PlayerBust || DealerBust)
  {
    NextPhase(&GameState->game_phase);
  }

  // RENDER
  {
    hand Hand = GameState->dealer;
    for (u32 Index = 0; Index < Hand.card_count; ++Index)
    {
      if (Index == 0 && (GameState->game_phase == START || GameState->game_phase == PLAYER))
      {
        PushQuad(Renderer, { 2.0f, 0.0f },
                 Mat4Translate(Index*Renderer->card_width*0.5f, Index*Renderer->card_height*0.5f + 100.0f, 1.0f));
      }
      else
      {
        PushQuad(Renderer, { (f32)Hand.cards[Index].type, (f32)Hand.cards[Index].suit },
                 Mat4Translate(Index*Renderer->card_width*0.5f, Index*Renderer->card_height*0.5f + 100.0f, 1.0f));
      }
    }
  }
  {
    hand Hand = GameState->player;
    for (u32 Index = 0; Index < Hand.card_count; ++Index)
    {
      PushQuad(Renderer, { (f32)Hand.cards[Index].type, (f32)Hand.cards[Index].suit },
               Mat4Translate(Index*Renderer->card_width*0.5f, Index*Renderer->card_height*0.5f - 100.0f, 1.0f));
    }
  }
}

