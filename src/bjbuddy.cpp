// TODO:
// General Player Actions
// - Surrendering/Insurance
// - NA rules seperation from Intl. rules
//   - EX: NA only rule where dealers get a hole card but
//     in the rest of the world they only get an up card.
//       - This means an option needs to be added to handle
//         regular play to continue.
// Stats
// - Currency
// - Count (running and true) - Will be done with Hi/Lo but might add other counts
// - Discard tray deck estimation
// - Based on bet spread, amount that should be wagered
// - Basic strategy play (with and without count and including EHNC)
// Settings (for type of game, location, etc)
// - Whether to show or hide double downs
//

#pragma once
#include "core.unity.h"

global u32 MAX_HAND_COUNT = 13;
global u32 MIN_BET_SIZE   = 10;
global u32 MAX_BET_SIZE   = 100;

static inline void PlaceBet(app_state *GameState, u32 BetSize = MIN_BET_SIZE)
{
  u32 *PlayerMoney = &GameState->player_money;
  NeoAssert(*PlayerMoney - BetSize >= 0);
  *PlayerMoney -= BetSize;
  GameState->player.wager = BetSize;
}

static inline void NextPhase(enum phase *Phase)
{
    *Phase = (enum phase)((s32)(*Phase) + 1);
}

static void Hit(deck *Deck, hand *Hand)
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

static inline void DoubleDown(deck *Deck, hand *Hand,
                              enum phase *Phase)
{
  Hit(Deck, Hand);
  Hand->cards[Hand->card_count - 1].is_dd = true;
  Hand->wager *= 2;
  // TODO: When split is implemented, will need to move on
  // to the next player's hand (if they have one) or to next player.
  NextPhase(Phase);
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
  GameState->player.wager = 0;
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

    // TODO: Need a period before the cards are dealt to
    // set bet amount for player.
    GameState->player_money = 5000;
    PlaceBet(GameState);

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

    // TODO: Handle paying out the player immediately
    // if they have BJ.

    NextPhase(&GameState->game_phase);

    Memory->is_init = true;
  }

  if (GameState->game_phase == START)
  {
    PlaceBet(GameState);

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
          else if (ActionRight.half_transitions != 0 && ActionRight.is_down)
          {
              NextPhase(&GameState->game_phase);
          }
          else if (ActionDown.half_transitions != 0 && ActionDown.is_down)
          {
            DoubleDown(&GameState->base_deck, &GameState->player, &GameState->game_phase);

            char OutStr[256];
            _snprintf_s(OutStr, sizeof(OutStr), "Hand Value: %d\n", GameState->player.value);
            OutputDebugStringA(OutStr);

            OutputDebugStringA("======================\n");
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
          else if (ActionRight.half_transitions != 0 && ActionRight.is_down)
          {
              NextPhase(&GameState->game_phase);
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

  mat4 CenterTranslate = Mat4Translate((f32)Renderer->width*0.5f, (f32)Renderer->height*0.5f, 0.0f);
#if 1
  vertex_data Vertices[] =
  {
    // pos                                                                             color               tex-coords
    { {200.0f, 100.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} }, // Bottom-Left
    { {(f32)Renderer->width - 200.0f, 100.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} }, // Bottom-Right
    { {(f32)Renderer->width - 200.0f, (f32)Renderer->height - 100.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} }, // Top-Right
    { {200.0f, (f32)Renderer->height - 100.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} }, // Top-Left
  };

  u32 EboIndexPattern[] =
  {
    0, 1, 3,
    1, 2, 3,
  };

  memory_arena *FrameArena = &Renderer->frame_arena;
  render_unit *Unit = PushStruct(FrameArena, render_unit);

  u32 EboIndexCount = ArrayCount(EboIndexPattern);
  u32 *Indices = PushArray(FrameArena, EboIndexCount, u32);
  Unit->indices = Indices;
  Unit->index_count = EboIndexCount;
  u32 UnitCount = Renderer->unit_count;
  memcpy(Unit->indices, EboIndexPattern, EboIndexCount*sizeof(u32));
  u32 VertexCount = ArrayCount(Vertices);
  Unit->vertices = PushArray(FrameArena, VertexCount, vertex_data);
  Unit->vertex_count = VertexCount;
  Unit->model = Mat4Iden();
  memcpy(Unit->vertices, Vertices, VertexCount*sizeof(vertex_data));
  Renderer->head = Unit;
  ++Renderer->unit_count;
#else
  {
    hand Hand = GameState->dealer;
    for (u32 Index = 0; Index < Hand.card_count; ++Index)
    {
      mat4 Transform = Mat4Translate(Index*Renderer->card_width*0.5f, Index*Renderer->card_height*0.5f + 100.0f, 1.0f)*CenterTranslate;
      if (Index == 0 && (GameState->game_phase == START || GameState->game_phase == PLAYER))
      {
        PushQuad(Renderer, { 2.0f, 0.0f }, Transform);
      }
      else
      {
        PushQuad(Renderer, { (f32)Hand.cards[Index].type, (f32)Hand.cards[Index].suit }, Transform);
      }
    }
  }
  {
    hand Hand = GameState->player;
    for (u32 Index = 0; Index < Hand.card_count; ++Index)
    {
      if (Hand.cards[Index].is_dd)
      {
        mat4 Transform = Mat4Translate(((Index - 1)*Renderer->card_width*0.5f) + Renderer->card_height, Index*Renderer->card_height*0.5f - 100.0f, 1.0f)*
                         CenterTranslate*
                         Mat4RotateZ(PI32 / 2.0f);
        PushQuad(Renderer, { (f32)Hand.cards[Index].type, (f32)Hand.cards[Index].suit }, Transform);
      }
      else
      {
        mat4 Transform = Mat4Translate(Index*Renderer->card_width*0.5f, Index*Renderer->card_height*0.5f - 100.0f, 1.0f)*CenterTranslate;
        PushQuad(Renderer, { (f32)Hand.cards[Index].type, (f32)Hand.cards[Index].suit }, Transform);
      }
    }
  }
#endif
}

