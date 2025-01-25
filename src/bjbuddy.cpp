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
  Hand->cards[Hand->card_count++] = DrawnCard;
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

// NOTE: players includes the dealer
inline static void NewHandDeal(deck *deck, u32 players = 2)
{
  // TODO: Pass the removed cards to the players/board;
  u32 cards_to_remove = players*2;
  // card *cards_removed;
  // for (u32 i = 0; i < cards_to_remove; ++i)
  // {
  //     cards_removed++ = &deck->cards[i];
  // }

  // return cards_removed;
}

#if 0
#include "neo_jpg.h"
#include "neo_jpg.cpp"
#endif

static void DrawCard(vertex_data *VertexArray, loaded_bmp TexAtlas, v2 CardIndex) {
  f32 CardWidth = (f32)TexAtlas.width / 13.0f;
  f32 CardHeight = (f32)TexAtlas.height / 5.0f;

  v2 ComputedTexCoords[] = {
    {(CardIndex.x * CardWidth) / TexAtlas.width, (CardIndex.y * CardHeight) / TexAtlas.height },
    {((CardIndex.x + 1) * CardWidth) / TexAtlas.width, (CardIndex.y * CardHeight) / TexAtlas.height },
    {((CardIndex.x + 1) * CardWidth) / TexAtlas.width, ((CardIndex.y + 1) * CardHeight) / TexAtlas.height },
    {(CardIndex.x * CardWidth) / TexAtlas.width, ((CardIndex.y + 1) * CardHeight) / TexAtlas.height }
  };
  for (u32 Index = 0; Index < 4; ++Index) {
    VertexArray[Index].tex_coords = ComputedTexCoords[Index];
  }
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
        { TWO, SPADES },
        { THREE, SPADES },
        { FOUR, SPADES },
        { FIVE, SPADES },
        { SIX, SPADES },
        { SEVEN, SPADES },
        { EIGHT, SPADES },
        { NINE, SPADES },
        { TEN, SPADES },
        { JACK, SPADES },
        { QUEEN, SPADES },
        { KING, SPADES },
        { ACE, SPADES },
        { TWO, HEARTS },
        { THREE, HEARTS },
        { FOUR, HEARTS },
        { FIVE, HEARTS },
        { SIX, HEARTS },
        { SEVEN, HEARTS },
        { EIGHT, HEARTS },
        { NINE, HEARTS },
        { TEN, HEARTS },
        { JACK, HEARTS },
        { QUEEN, HEARTS },
        { KING, HEARTS },
        { ACE, HEARTS },
        { TWO, CLUBS },
        { THREE, CLUBS },
        { FOUR, CLUBS },
        { FIVE, CLUBS },
        { SIX, CLUBS },
        { SEVEN, CLUBS },
        { EIGHT, CLUBS },
        { NINE, CLUBS },
        { TEN, CLUBS },
        { JACK, CLUBS },
        { QUEEN, CLUBS },
        { KING, CLUBS },
        { ACE, CLUBS },
        { TWO, DIAMONDS },
        { THREE, DIAMONDS },
        { FOUR, DIAMONDS },
        { FIVE, DIAMONDS },
        { SIX, DIAMONDS },
        { SEVEN, DIAMONDS },
        { EIGHT, DIAMONDS },
        { NINE, DIAMONDS },
        { TEN, DIAMONDS },
        { JACK, DIAMONDS },
        { QUEEN, DIAMONDS },
        { KING, DIAMONDS },
        { ACE, DIAMONDS }
      },
    };

    // Blackjack Setup

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

    Memory->is_init = true;
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
      else if (ActionLeft.half_transitions != 0 && ActionLeft.is_down)
      {
        Hit(&GameState->base_deck, &GameState->player);
      }
    }
  }

  {
    hand Hand = GameState->dealer;
    for (u32 Index = 0; Index < Hand.card_count; ++Index)
    {
      if (Index == 0)
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

