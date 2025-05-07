// TODO:
// General Player Actions
// - Surrendering/Insurance
// - NA rules seperation from Intl. rules
//   - Currency for other regions
//   - EX: NA only rule where dealers get a hole card but
//     in the rest of the world they only get an up card.
//       - This means an option needs to be added to handle
//         regular play to continue.
// Stats
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

internal void PayOut(f32 *Bankroll, f32 Wager, b32 IsBlackjack = false)
{
  // TODO: Settings
  if (IsBlackjack)
  {
    // 3:2 - includes the original wager
    *Bankroll += 1.5f*Wager;
  }
  *Bankroll += Wager;
}
internal void PlaceBet(f32 *Bankroll, hand *Hand, f32 Wager = MIN_BET_SIZE)
{
  // TODO: Show something if the player does this but does not have the funds.
  NeoAssert(*Bankroll - Wager >= 0);
  Hand->wager += Wager;
}

internal void NextPhase(enum phase *Phase)
{
    *Phase = (enum phase)((s32)(*Phase) + 1);
}

internal b32 CheckBust(u32 *HandValue)
{
  b32 Result = *HandValue > 21;
  return Result;
}

internal void Hit(deck *Deck, hand *Hand, s32 *RCount, b32 IsDoubleDown = false)
{
  NeoAssert(Deck->current - Deck->cards <= 52);
  // TODO: Do something if we actually hit this.
  if (Hand->card_count >= 13) return;
  card DrawnCard = *Deck->current++;
  switch (DrawnCard.rank)
  {
    case TEN:
    case ACE:
    {
      *RCount -= 1;
    } break;
    case TWO:
    case THREE:
    case FOUR:
    case FIVE:
    case SIX:
    {
      *RCount += 1;
    } break;
    default: {} break;
  }

  // TODO: Need to handle making the ACE valued at 11 again
  // in the split function.
  if (DrawnCard.rank == ACE && (Hand->value + DrawnCard.value) > 21)
  {
    DrawnCard.value = 1;
  }

  Hand->cards[Hand->card_count++] = DrawnCard;
  if (IsDoubleDown)
  {
    Hand->cards[Hand->card_count - 1].is_dd = true;
  }
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

// TODO: Should I keep this? If so I need to rework
// how to convert it over
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
    // case JACK: return "JACK";
    // case QUEEN: return "QUEEN";
    // case KING: return "KING";
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

internal deck CreateDeck()
{
  deck Result = {
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
    return Result;
}

internal void ResetRound(app_state *GameState)
{
  player *Ap = &GameState->ap;
  hand *Dealer = &GameState->dealer;

  Dealer->card_count = 0;
  Dealer->value = 0;

  for (u32 Idx = 0; Idx < Ap->hand_count; ++Idx)
  {
    hand *Hand = &Ap->hands[Idx];
    Hand->card_count = 0;
    Hand->value = 0;
    Hand->wager = 0;
  }
  Ap->hand_idx = 0;
  Ap->hand_count = 1;
}

static void UpdateAndRender(thread_context *Thread, app_memory *Memory, engine_input *Input, renderer *Renderer)
{
  NeoAssert(sizeof(app_state) <= Memory->perm_storage_size);
  app_state *GameState = (app_state *)Memory->perm_memory;

  static b32 FirstRound = true;
  if(!Memory->is_init)
  {
    InitArena(&GameState->core_arena, Memory->perm_storage_size - sizeof(app_state),
              (u8 *)Memory->perm_memory + sizeof(app_state));

    // Blackjack Setup

    GameState->game_phase = NULL_PHASE;
    // TODO: Setting
    shoe Shoe = {0};
    Shoe.decks = PushArray(&GameState->core_arena, Shoe.deck_count, deck);
    for (u32 Idx = 0; Idx < Shoe.deck_count; ++Idx)
    {
      Shoe.decks[Idx] = CreateDeck();
      for (s32 ShuffleCount = 0; ShuffleCount < 4; ++ShuffleCount)
      {
        Shuffle(Shoe.decks[Idx].cards, ArrayCount(Shoe.decks[Idx].cards));
      }
    }

    // TODO: Need a period before the cards are dealt to
    // set bet amount for player.
    player Ap = {0};
    Ap.bankroll = 10000;

    // TODO: Make burning a card an option/setting.
    Shoe.decks[0].current = &Shoe.decks[0].cards[1];

    // TODO: Make settings for number of allowed hands
    Ap.hands = PushArray(&GameState->core_arena, 5, hand);
    for (s32 Idx = 0; Idx < 5; ++Idx)
    {
      Ap.hands[Idx].cards = PushArray(&GameState->core_arena, MAX_HAND_COUNT, card);
    }
    Ap.hand_count = 1;

    hand DealerHand = {0};
    DealerHand.cards = PushArray(&GameState->core_arena, MAX_HAND_COUNT, card);

    GameState->shoe = Shoe;
    GameState->dealer = DealerHand;
    GameState->ap = Ap;

    Memory->is_init = true;
  }

  // UPDATE
  deck *ActiveDeck = &GameState->shoe.decks[0];
  if (GameState->game_phase == END)
  {
    ResetRound(GameState);
    GameState->game_phase = NULL_PHASE;
    if (FirstRound) FirstRound = false;
    ActiveDeck->discarded = (ActiveDeck->current - ActiveDeck->cards) - 1;
    // TODO: Setup deck division choices (half deck, quarter, eighth, etc.).
    // NOTE: These are the typical ways of rounding the true count. I will likely use floor
    // but will leave it for now until I get the sim working.
    //   - Truncate – For positive numbers, round down and for negative numbers round up. This is the method used in the 1994 and later editions of Professional Blackjack.
    //     1.5 is rounded down to 1. -1.5 is rounded up to -1.
    //   - Floor – Numbers are always rounded down. This is the method used in the early versions of Professional Blackjack.
    //     1.5 is rounded down to 1. -1.5 is rounded down to -2. This is the most popular method used now.
    //   - Round – After the True Count division, the result is rounded to the nearest integer.
    //     If the number is exactly between two integers, it is rounded up. 1.5 is rounded up to 2. -1.5 is rounded up to -1. This method is also common.
    //   - Statistical Round – After the True Count division, the result is rounded to the nearest integer.
    //     If the number is exactly between two integers, it is rounded to the nearest even number. 1.5 is rounded up to 2. -1.5 is rounded down to -2.
    // TODO: Update to be used for all decks remaining
    if (GameState->shoe.deck_count > 1 && ActiveDeck->discarded != 52)
    {
      GameState->true_count = (f32)GameState->running_count / (f32)(52 - ActiveDeck->discarded);
    }
    else
    {
      GameState->true_count = (f32)GameState->running_count;
    }
  }

  hand *DealerHand = &GameState->dealer;
  hand *PlayerHand = &GameState->ap.hands[GameState->ap.hand_idx];

  if (GameState->game_phase == START)
  {
    PlaceBet(&GameState->ap.bankroll, PlayerHand);

    Hit(ActiveDeck, PlayerHand, &GameState->running_count);
    Hit(ActiveDeck, DealerHand, &GameState->running_count);
    Hit(ActiveDeck, PlayerHand, &GameState->running_count);
    Hit(ActiveDeck, DealerHand, &GameState->running_count);

    b32 AllBJs = true;
    for (u32 Idx = 0; Idx < GameState->ap.hand_count; ++Idx)
    {
      hand *Hand = &GameState->ap.hands[Idx];
      if (Hand->value == 21)
      {
        PayOut(&GameState->ap.bankroll, Hand->wager);
        Hand->card_count = 0;
        Hand->value = 0;
        Hand->wager = 0;
      }
      else
      {
         AllBJs = false;
      }
    }

    if (AllBJs)
    {
      GameState->game_phase = END;
    }
    else
    {
      NextPhase(&GameState->game_phase);
    }
  }

  // NOTE: For cases when a hand pays out BJ and exists inbetween all the hands
  // ie. players has 3 hands and the middle hand played out BJ, the active player
  // hand should be updated accordingly before processing player actions.
  for (; GameState->game_phase != NULL_PHASE && PlayerHand->value == 0; PlayerHand = &GameState->ap.hands[++GameState->ap.hand_idx]);

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
        case NULL_PHASE:
        {
          if (ActionRight.half_transitions != 0 && ActionRight.is_down)
          {
            char OutStr[256];
            card *Cards = ActiveDeck->current;
            OutputDebugStringA("Deck (current)\n");
            for (u32 Index = 0; Index < 4; ++Index)
            {
              _snprintf_s(OutStr, sizeof(OutStr), "RANK: %s SUIT: %s\n",
                          TypeToCStr(Cards[Index].rank), SuitToCStr(Cards[Index].suit));
              OutputDebugStringA(OutStr);
            }
            OutputDebugStringA("======================\n");
          }
          if (ActionDown.half_transitions != 0 && ActionDown.is_down)
          {
            char OutStr[256];
            card *Cards = ActiveDeck->current;
            OutputDebugStringA("Deck (current)\n");
            for (u32 Index = 0; Index < 4; ++Index)
            {
              _snprintf_s(OutStr, sizeof(OutStr), "RANK: %s SUIT: %s\n",
                          TypeToCStr(Cards[Index].rank), SuitToCStr(Cards[Index].suit));
              OutputDebugStringA(OutStr);
            }
            OutputDebugStringA("======================\n");

            NextPhase(&GameState->game_phase);
          }
        } break;
        case PLAYER:
        {
          if (ActionLeft.half_transitions != 0 && ActionLeft.is_down)
          {
            // TODO: Bake busting into hit func?
            Hit(ActiveDeck, PlayerHand, &GameState->running_count);
            b32 Busted = CheckBust(&PlayerHand->value);
            if (Busted && (++GameState->ap.hand_idx == GameState->ap.hand_count))
            {
              --GameState->ap.hand_idx;
              NextPhase(&GameState->game_phase);
            }

            char OutStr[256];
            _snprintf_s(OutStr, sizeof(OutStr), "Hand Value: %d\n", PlayerHand->value);
            OutputDebugStringA(OutStr);

            OutputDebugStringA("======================\n");
          }
          else if (ActionRight.half_transitions != 0 && ActionRight.is_down)
          {
            player *Player = &GameState->ap;
            if (++Player->hand_idx == Player->hand_count)
            {
              --Player->hand_idx;
              NextPhase(&GameState->game_phase);
            }
          }
          else if (ActionDown.half_transitions != 0 && ActionDown.is_down)
          {
            if (PlayerHand->card_count == 2)
            {
              PlaceBet(&GameState->ap.bankroll, PlayerHand, PlayerHand->wager);
              Hit(ActiveDeck, PlayerHand, &GameState->running_count, true);
              NextPhase(&GameState->game_phase);
            }

            char OutStr[256];
            _snprintf_s(OutStr, sizeof(OutStr), "Hand Value: %d\n", PlayerHand->value);
            OutputDebugStringA(OutStr);

            OutputDebugStringA("======================\n");
          }
        } break;
        case DEALER:
        {
          if (ActionLeft.half_transitions != 0 && ActionLeft.is_down)
          {
            // TODO: Bake busting into hit func?
            Hit(ActiveDeck, &GameState->dealer, &GameState->running_count);
            // TODO: Add this back once I setup the 'flow' between phases.
            // if (CheckBust(&DealerHand->value)) NextPhase(&GameState->game_phase);

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

        default: {}
      }
    }
  }

  // Evaluate current player hand
  if (GameState->game_phase == EVAL)
  {
    if (CheckBust(&DealerHand->value) ||
       (!CheckBust(&PlayerHand->value) && DealerHand->value < PlayerHand->value))
    {
      PayOut(&GameState->ap.bankroll, PlayerHand->wager);
    }
    else if (CheckBust(&PlayerHand->value) || DealerHand->value > PlayerHand->value)
    {
      GameState->ap.bankroll -= PlayerHand->wager;
    }

    NextPhase(&GameState->game_phase);
  }


  // RENDER
  ResetRenderer(Renderer);
  mat4 CenterTranslate = Mat4Translate((f32)Renderer->width*0.5f, (f32)Renderer->height*0.5f, 0.0f);
  {
    // Dealer's Cards
    hand Hand = GameState->dealer;
    for (u32 Index = 0; Index < Hand.card_count; ++Index)
    {
      mat4 Transform = Mat4Translate(Index*Renderer->card_width*0.5f, Index*Renderer->card_height*0.5f + 200.0f, 1.0f)*CenterTranslate;
      if (Index == 0 && (GameState->game_phase == START || GameState->game_phase == PLAYER))
      {
        PushCard(Renderer, { 2.0f, 0.0f }, Transform);
      }
      else
      {
        PushCard(Renderer, { (f32)Hand.cards[Index].rank, (f32)Hand.cards[Index].suit }, Transform);
      }
    }
  }
  {
    // Player's Cards
    for (u32 HCount = 0; HCount < GameState->ap.hand_count; ++HCount)
    {
      hand Hand = GameState->ap.hands[HCount];
      // TODO: Update the transforms to account for more than one hand
      for (u32 Index = 0; Index < Hand.card_count; ++Index)
      {
        if (Hand.cards[Index].is_dd)
        {
          mat4 Transform = Mat4Translate(((Index - 1)*Renderer->card_width*0.5f) + Renderer->card_height, Index*Renderer->card_height*0.5f - 200.0f, 1.0f)*
            CenterTranslate*
            Mat4RotateZ(PI32 / 2.0f);
          PushCard(Renderer, { (f32)Hand.cards[Index].rank, (f32)Hand.cards[Index].suit }, Transform);
        }
        else
        {
          mat4 Transform = Mat4Translate(Index*Renderer->card_width*0.5f, Index*Renderer->card_height*0.5f - 200.0f, 1.0f)*CenterTranslate;
          PushCard(Renderer, { (f32)Hand.cards[Index].rank, (f32)Hand.cards[Index].suit }, Transform);
        }
      }
    }
  }
  {
    // 'Discard Tray'
    // TODO: I feel like there should be a way to reuse vertex data instead of pushing an identical card.
    if (ActiveDeck->discarded > 0 && !FirstRound)
    {
      mat4 CardTransform = Mat4Translate((Renderer->card_width*0.5f) + Renderer->card_height,
                                     Renderer->card_height*0.5f + Renderer->width*0.25f, 1.0f)*Mat4RotateZ(PI32 / 4.0f);
      PushCard(Renderer, { 2.0f, 0.0f }, CardTransform);
      char TextContainer[256];
      // Cards in discard tray
      _snprintf_s(TextContainer, sizeof(TextContainer), "Discarded: %llu", ActiveDeck->discarded);
      PushText(Renderer, StrFromCStr(TextContainer), Mat4Translate(5.0f, Renderer->width*0.25f + 150.0f, 0.0f)*Mat4Scale(0.65f, 0.65f, 1.0f));
    }
  }
  {
    char TextContainer[256];

    mat4 TextTransform = Mat4Translate(5.0f, (f32)Renderer->height - 40.0f, 0.0f)*Mat4Scale(0.65f, 0.65f, 1.0f);
    string Lines[2] = {0};

    // Running Count
    _snprintf_s(TextContainer, sizeof(TextContainer), "Running Count: %d", GameState->running_count);
    Lines[0] = StrAllocFromCStr(TextContainer);
    // True Count
    _snprintf_s(TextContainer, sizeof(TextContainer), "True Count: %.3f", GameState->true_count);
    Lines[1] = StrAllocFromCStr(TextContainer);

    // FIX: PIGGY AF PLZ FIX SOON!!!
    PushLinesOfText(Renderer, Lines, ArrayCount(Lines), TextTransform);
    FreeStrAlloc(&Lines[0]);
    FreeStrAlloc(&Lines[1]);

    // Bankroll
    _snprintf_s(TextContainer, sizeof(TextContainer), "Bankroll: $%.2f", GameState->ap.bankroll);
    PushText(Renderer, StrFromCStr(TextContainer), Mat4Translate(5.0f, 10.0f, 0.0f)*Mat4Scale(0.65f, 0.65f, 1.0f));
    // Wager
    _snprintf_s(TextContainer, sizeof(TextContainer), "Wager: $%.2f", PlayerHand->wager);
    PushText(Renderer, StrFromCStr(TextContainer), CenterTranslate*Mat4Translate(-175.0f, -(f32)Renderer->height*0.5f + 10.0f, 0.0f)*Mat4Scale(0.65f, 0.65f, 1.0f));
  }
}

