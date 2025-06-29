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

internal void ResetGameState(app_state *GameState, app_memory *Memory, scene Scene)
{
  memset(GameState, 0, sizeof(app_state));
  InitArena(&GameState->core_arena, Memory->perm_storage_size - sizeof(app_state),
            (u8 *)Memory->perm_memory + sizeof(app_state));
  GameState->scene = Scene;
}

internal void BackToMenu(engine_controller_input *Controller, app_state *GameState)
{
  engine_button_state Back = Controller->back;
  if (Back.half_transitions != 0 && Back.is_down)
  {
    GameState->scene = s_MENU;
    GameState->scene_initialized = false;
  }
}

internal void RunMenuScene(app_state *GameState, engine_input *Input, renderer *Renderer)
{
  static s32 g_CurrentSelection = 0;
  Renderer->clear_color = V3(0.47f, 0.13f, 0.17f);

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
      engine_button_state Start = Controller->start;
      if (Start.half_transitions != 0 && Start.is_down)
      {
        if (g_CurrentSelection == 0)
        {
          GameState->scene = s_GAME;
          return;
        }
        else if (g_CurrentSelection == 1)
        {
          // TODO: Update GameState to hold necessary settings for this mode.
          GameState->scene = s_GAME;
          return;
        }
        else if (g_CurrentSelection == 2)
        {
          GameState->scene = s_SIMU;
          return;
        }
        else if (g_CurrentSelection == 3)
        {
          GameState->scene = s_QUIT;
          return;
        }
      }
      engine_button_state ActionDown = Controller->action_down;
      if (ActionDown.half_transitions != 0 && ActionDown.is_down)
      {
        g_CurrentSelection += 1;
        if (g_CurrentSelection == 4)
        {
          g_CurrentSelection = 0;
        }
      }
      engine_button_state ActionUp = Controller->action_up;
      if (ActionUp.half_transitions != 0 && ActionUp.is_down)
      {
        g_CurrentSelection -= 1;
        if (g_CurrentSelection == -1)
        {
          g_CurrentSelection = 3;
        }
      }
    }
  }

  ResetRenderer(Renderer);
  {
    mat4 CenterTextXform = Mat4Translate((f32)Renderer->width*0.5f - (15.0f*15.0f), (f32)Renderer->height*0.85f, 0.0f)*Mat4Scale(0.90f, 0.90f, 1.0f);
    PushText(Renderer, StrFromCStr("Blackjack Buddy"), CenterTextXform);
  }
  {
    mat4 CenterTextXform = Mat4Translate((f32)Renderer->width*0.5f - (15.0f*15.0f) - 115.0f, (f32)Renderer->height*0.65f, 0.0f)*Mat4Scale(0.80f, 0.80f, 1.0f);
    v3 Color = {0.5f, 0.5f, 0.5f};
    if (g_CurrentSelection == 0)
    {
      Color = {1.0f, 1.0f, 1.0f};
    }
    PushText(Renderer, StrFromCStr("Basic Strategy Training"), CenterTextXform, Color);
  }
  {
    mat4 CenterTextXform = Mat4Translate((f32)Renderer->width*0.5f - (15.0f*15.0f) - 30.0f, (f32)Renderer->height*0.56f, 0.0f)*Mat4Scale(0.80f, 0.80f, 1.0f);
    v3 Color = {0.5f, 0.5f, 0.5f};
    if (g_CurrentSelection == 1)
    {
      Color = {1.0f, 1.0f, 1.0f};
    }
    PushText(Renderer, StrFromCStr("Counting Training"), CenterTextXform, Color);
  }
  {
    mat4 CenterTextXform = Mat4Translate((f32)Renderer->width*0.5f - (15.0f*15.0f), (f32)Renderer->height*0.47f, 0.0f)*Mat4Scale(0.80f, 0.80f, 1.0f);
    v3 Color = {0.5f, 0.5f, 0.5f};
    if (g_CurrentSelection == 2)
    {
      Color = {1.0f, 1.0f, 1.0f};
    }
    PushText(Renderer, StrFromCStr("Bankroll & Sim"), CenterTextXform, Color);
  }
  {
    mat4 CenterTextXform = Mat4Translate((f32)Renderer->width*0.5f - (15.0f*5.0f), (f32)Renderer->height*0.38f, 0.0f)*Mat4Scale(0.80f, 0.80f, 1.0f);
    v3 Color = {0.5f, 0.5f, 0.5f};
    if (g_CurrentSelection == 3)
    {
      Color = {1.0f, 1.0f, 1.0f};
    }
    PushText(Renderer, StrFromCStr("Quit"), CenterTextXform, Color);
  }
  // {
  //   char TextContainer[256];
  //
  //   mat4 TextTransform = Mat4Translate(5.0f, (f32)Renderer->height - 40.0f, 0.0f)*Mat4Scale(0.65f, 0.65f, 1.0f);
  //   string Lines[2] = {0};
  //
  //   // Running Count
  //   _snprintf_s(TextContainer, sizeof(TextContainer), "Running Count: %d", GameState->running_count);
  //   Lines[0] = StrAllocFromCStr(TextContainer);
  //   // True Count
  //   _snprintf_s(TextContainer, sizeof(TextContainer), "True Count: %.3f", GameState->true_count);
  //   Lines[1] = StrAllocFromCStr(TextContainer);
  //
  //   // FIX: PIGGY AF PLZ FIX SOON!!!
  //   PushLinesOfText(Renderer, Lines, ArrayCount(Lines), TextTransform);
  //   FreeStrAlloc(&Lines[0]);
  //   FreeStrAlloc(&Lines[1]);
  //
  //   // Bankroll
  //   _snprintf_s(TextContainer, sizeof(TextContainer), "Bankroll: $%.2f", GameState->ap.bankroll);
  //   PushText(Renderer, StrFromCStr(TextContainer), Mat4Translate(5.0f, 10.0f, 0.0f)*Mat4Scale(0.65f, 0.65f, 1.0f));
  // }
}

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

internal inline b32 CheckBust(u32 *HandValue)
{
  b32 Result = *HandValue > 21;
  return Result;
}

internal void Hit(shoe *Shoe, hand *Hand, s32 *RCount, b32 IsDoubleDown = false)
{
  NeoAssert(Shoe->current - Shoe->cards <= Shoe->card_count);
  // TODO: Do something if we actually hit this.
  if (Hand->card_count >= 13) InvalidCodePath;
  card DrawnCard = *Shoe->current++;
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
internal inline void Shuffle(card *Cards, ums CardCount)
{
  u32 RandomIndex = 0;
  for (ums Index = 0; Index <= CardCount - 1; ++Index)
  {
    ums SwapIndex = RandomNumberTable[RandomIndex++] % (CardCount - 1);
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
internal inline char *TypeToCStr(s32 CardType)
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
internal inline char *SuitToCStr(s32 CardSuit)
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

internal inline void CreateDeck(shoe *Shoe, u32 DeckIdx)
{
  Shoe->cards[(52*DeckIdx) + 0] = { TWO, SPADES, {1, 1}, 2 };
  Shoe->cards[(52*DeckIdx) + 1] = { THREE, SPADES, {2, 1}, 3 };
  Shoe->cards[(52*DeckIdx) + 2] = { FOUR, SPADES, {3, 1}, 4 };
  Shoe->cards[(52*DeckIdx) + 3] = { FIVE, SPADES, {4, 1}, 5 };
  Shoe->cards[(52*DeckIdx) + 4] = { SIX, SPADES, {5, 1}, 6 };
  Shoe->cards[(52*DeckIdx) + 5] = { SEVEN, SPADES, {6, 1}, 7 };
  Shoe->cards[(52*DeckIdx) + 6] = { EIGHT, SPADES, {7, 1}, 8 };
  Shoe->cards[(52*DeckIdx) + 7] = { NINE, SPADES, {8, 1}, 9 };
  Shoe->cards[(52*DeckIdx) + 8] = { TEN, SPADES, {9, 1}, 10 };
  Shoe->cards[(52*DeckIdx) + 9] = { JACK, SPADES, {10, 1}, 10 };
  Shoe->cards[(52*DeckIdx) + 10] = { QUEEN, SPADES, {11, 1}, 10 };
  Shoe->cards[(52*DeckIdx) + 11] = { KING, SPADES, {12, 1}, 10 };
  Shoe->cards[(52*DeckIdx) + 12] = { ACE, SPADES, {0, 1}, 11 };
  Shoe->cards[(52*DeckIdx) + 13] = { TWO, HEARTS, {1, 2}, 2 };
  Shoe->cards[(52*DeckIdx) + 14] = { THREE, HEARTS, {2, 2}, 3 };
  Shoe->cards[(52*DeckIdx) + 15] = { FOUR, HEARTS, {3, 2}, 4 };
  Shoe->cards[(52*DeckIdx) + 16] = { FIVE, HEARTS, {4, 2}, 5 };
  Shoe->cards[(52*DeckIdx) + 17] = { SIX, HEARTS, {5, 2}, 6 };
  Shoe->cards[(52*DeckIdx) + 18] = { SEVEN, HEARTS, {6, 2}, 7 };
  Shoe->cards[(52*DeckIdx) + 19] = { EIGHT, HEARTS, {7, 2}, 8 };
  Shoe->cards[(52*DeckIdx) + 20] = { NINE, HEARTS, {8, 2}, 9 };
  Shoe->cards[(52*DeckIdx) + 21] = { TEN, HEARTS, {9, 2}, 10 };
  Shoe->cards[(52*DeckIdx) + 22] = { JACK, HEARTS, {10, 2}, 10 };
  Shoe->cards[(52*DeckIdx) + 23] = { QUEEN, HEARTS, {11, 2}, 10 };
  Shoe->cards[(52*DeckIdx) + 24] = { KING, HEARTS, {12, 2}, 10 };
  Shoe->cards[(52*DeckIdx) + 25] = { ACE, HEARTS, {0, 2}, 11 };
  Shoe->cards[(52*DeckIdx) + 26] = { TWO, CLUBS, {1, 4}, 2 };
  Shoe->cards[(52*DeckIdx) + 27] = { THREE, CLUBS, {2, 4}, 3 };
  Shoe->cards[(52*DeckIdx) + 28] = { FOUR, CLUBS, {3, 4}, 4 };
  Shoe->cards[(52*DeckIdx) + 29] = { FIVE, CLUBS, {4, 4}, 5 };
  Shoe->cards[(52*DeckIdx) + 30] = { SIX, CLUBS, {5, 4}, 6 };
  Shoe->cards[(52*DeckIdx) + 31] = { SEVEN, CLUBS, {6, 4}, 7 };
  Shoe->cards[(52*DeckIdx) + 32] = { EIGHT, CLUBS, {7, 4}, 8 };
  Shoe->cards[(52*DeckIdx) + 33] = { NINE, CLUBS, {8, 4}, 9 };
  Shoe->cards[(52*DeckIdx) + 34] = { TEN, CLUBS, {9, 4}, 10 };
  Shoe->cards[(52*DeckIdx) + 35] = { JACK, CLUBS, {10, 4}, 10 };
  Shoe->cards[(52*DeckIdx) + 36] = { QUEEN, CLUBS, {11, 4}, 10 };
  Shoe->cards[(52*DeckIdx) + 37] = { KING, CLUBS, {12, 4}, 10 };
  Shoe->cards[(52*DeckIdx) + 38] = { ACE, CLUBS, {0, 4}, 11 };
  Shoe->cards[(52*DeckIdx) + 39] = { TWO, DIAMONDS, {1, 3}, 2 };
  Shoe->cards[(52*DeckIdx) + 40] = { THREE, DIAMONDS, {2, 3}, 3 };
  Shoe->cards[(52*DeckIdx) + 41] = { FOUR, DIAMONDS, {3, 3}, 4 };
  Shoe->cards[(52*DeckIdx) + 42] = { FIVE, DIAMONDS, {4, 3}, 5 };
  Shoe->cards[(52*DeckIdx) + 43] = { SIX, DIAMONDS, {5, 3}, 6 };
  Shoe->cards[(52*DeckIdx) + 44] = { SEVEN, DIAMONDS, {6, 3}, 7 };
  Shoe->cards[(52*DeckIdx) + 45] = { EIGHT, DIAMONDS, {7, 3}, 8 };
  Shoe->cards[(52*DeckIdx) + 46] = { NINE, DIAMONDS, {8, 3}, 9 };
  Shoe->cards[(52*DeckIdx) + 47] = { TEN, DIAMONDS, {9, 3}, 10 };
  Shoe->cards[(52*DeckIdx) + 48] = { JACK, DIAMONDS, {10, 3}, 10 };
  Shoe->cards[(52*DeckIdx) + 49] = { QUEEN, DIAMONDS, {11, 3}, 10 };
  Shoe->cards[(52*DeckIdx) + 50] = { KING, DIAMONDS, {12, 3}, 10 };
  Shoe->cards[(52*DeckIdx) + 51] = { ACE, DIAMONDS, {0, 3}, 11 };
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
  Ap->hand_count = 0;
}

internal void RunGameScene(app_state *GameState, engine_input *Input, renderer *Renderer)
{
  if (!GameState->scene_initialized)
  {
    GameState->scene_initialized = true;

    // Default table rules
    table_rules *TableRules = &GameState->table_rules;
    TableRules->number_of_decks = 2;
    TableRules->penetration = 1.5f;
    TableRules->h17 = true;
    TableRules->resplit_aces = ACE_NO_RESPLITTING;
    TableRules->max_hands = 4;
    TableRules->surrender = false;

    // Blackjack Setup

    GameState->game_phase = NULL_PHASE;
    // TODO: Setting
    shoe Shoe = {0};
    Shoe.deck_count = 6;
    Shoe.card_count = Shoe.deck_count*52;
    Shoe.cards = PushArray(&GameState->core_arena, Shoe.card_count, card);
    for (u32 DeckIdx = 0; DeckIdx < Shoe.deck_count; ++DeckIdx)
    {
      CreateDeck(&Shoe, DeckIdx);
    }
    for (s32 ShuffleCount = 0; ShuffleCount < 4; ++ShuffleCount)
    {
      Shuffle(Shoe.cards, Shoe.card_count);
    }
    // TODO: Setting - Penetration
    Shoe.cut_card = 52*Shoe.deck_count;

    // TODO: Need a period before the cards are dealt to
    // set bet amount for player.
    player Ap = {0};
    Ap.bankroll = 10000;

    // TODO: Make burning a card an option/setting.
    Shoe.current = &Shoe.cards[1];

    // TODO: Make settings for number of allowed hands
    Ap.hands = PushArray(&GameState->core_arena, 7, hand);
    for (s32 Idx = 0; Idx < 7; ++Idx)
    {
      Ap.hands[Idx].cards = PushArray(&GameState->core_arena, MAX_HAND_COUNT, card);
    }

    hand DealerHand = {0};
    DealerHand.cards = PushArray(&GameState->core_arena, MAX_HAND_COUNT, card);

    GameState->shoe = Shoe;
    GameState->dealer = DealerHand;
    GameState->ap = Ap;
  }

  static b32 FirstRound = true;

  // UPDATE
  shoe *Shoe = &GameState->shoe;
  if (GameState->game_phase == END)
  {
    ResetRound(GameState);
    GameState->game_phase = NULL_PHASE;
    if (FirstRound) FirstRound = false;
    // TODO: Only subtract one if a card is burned probably get out the bit from the setting.
    Shoe->discarded = (u32)(Shoe->current - Shoe->cards) - 1;
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
    if (Shoe->deck_count > 1)
    {
      f32 DecksRemaining = (f32)(Shoe->card_count - Shoe->discarded) / 52.0f;
      GameState->true_count = (f32)GameState->running_count / DecksRemaining;
    }
    else
    {
      GameState->true_count = (f32)GameState->running_count;
    }

    if (Shoe->current - Shoe->cards >= Shoe->cut_card)
    {
      // TODO: Reset Shoe
      InvalidCodePath;
    }
  }

  hand *DealerHand = &GameState->dealer;
  hand *PlayerHand = &GameState->ap.hands[GameState->ap.hand_idx];

  if (GameState->game_phase == START)
  {
    ++GameState->ap.hand_count;
    PlaceBet(&GameState->ap.bankroll, PlayerHand);

    Hit(Shoe, PlayerHand, &GameState->running_count);
    Hit(Shoe, DealerHand, &GameState->running_count);
    Hit(Shoe, PlayerHand, &GameState->running_count);
    Hit(Shoe, DealerHand, &GameState->running_count);

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

  // NOTE: We have moved on to the next player hand from a split
  if (GameState->game_phase == PLAYER && PlayerHand->card_count == 1) Hit(Shoe, PlayerHand, &GameState->running_count);

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
      BackToMenu(Controller, GameState);
      engine_button_state ActionDown = Controller->action_down;
      engine_button_state ActionUp = Controller->action_up;
      engine_button_state ActionRight = Controller->action_right;
      engine_button_state ActionLeft = Controller->action_left;
      engine_button_state RShoulder = Controller->right_shoulder;

      switch (GameState->game_phase)
      {
        case NULL_PHASE:
        {
          if (ActionDown.half_transitions != 0 && ActionDown.is_down)
          {
            NextPhase(&GameState->game_phase);
          }
        } break;
        case PLAYER:
        {
          if (ActionLeft.half_transitions != 0 && ActionLeft.is_down)
          {
            // TODO: Bake busting into hit func?
            player *Player = &GameState->ap;
            Hit(Shoe, PlayerHand, &GameState->running_count);

            b32 Busted = CheckBust(&PlayerHand->value);
            if (Busted)
            {
              if (((GameState->ap.hand_idx + 1) == GameState->ap.hand_count))
              {
                NextPhase(&GameState->game_phase);
              }
              else
              {
                ++Player->hand_idx;
              }
            }

            char OutStr[256];
            _snprintf_s(OutStr, sizeof(OutStr), "Hand Value: %d\n", PlayerHand->value);
            OutputDebugStringA(OutStr);

            OutputDebugStringA("======================\n");
          }
          else if (ActionRight.half_transitions != 0 && ActionRight.is_down)
          {
            player *Player = &GameState->ap;
            if ((Player->hand_idx + 1) == Player->hand_count)
            {
              NextPhase(&GameState->game_phase);
            }
            else
            {
              ++Player->hand_idx;
            }
          }
          else if (ActionDown.half_transitions != 0 && ActionDown.is_down)
          {
            if (PlayerHand->card_count == 2)
            {
              // TODO: Verify bet amount requirements for double down.
              player *Player = &GameState->ap;
              PlaceBet(&Player->bankroll, PlayerHand, PlayerHand->wager);
              Hit(Shoe, PlayerHand, &GameState->running_count, true);
              if ((Player->hand_idx + 1) == Player->hand_count)
              {
                NextPhase(&GameState->game_phase);
              }
              else
              {
                ++Player->hand_idx;
              }
            }

            char OutStr[256];
            _snprintf_s(OutStr, sizeof(OutStr), "Hand Value: %d\n", PlayerHand->value);
            OutputDebugStringA(OutStr);

            OutputDebugStringA("======================\n");
          }
          else if (ActionUp.half_transitions != 0 && ActionUp.is_down)
          {
            // NOTE: The splitting will default to one card only when splitting aces and no RSA.
            if (PlayerHand->card_count == 2 && PlayerHand->cards[0].rank == PlayerHand->cards[1].rank)
            {
              player *Player = &GameState->ap;
              hand *NewHand = &Player->hands[Player->hand_count++];
              NewHand->cards[0] = PlayerHand->cards[1];
              NewHand->value += NewHand->cards[0].value;
              PlayerHand->value -= PlayerHand->cards[1].value;
              ++NewHand->card_count;
              --PlayerHand->card_count;
              PlaceBet(&GameState->ap.bankroll, NewHand, PlayerHand->wager);
              Hit(Shoe, PlayerHand, &GameState->running_count);
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
            Hit(Shoe, &GameState->dealer, &GameState->running_count);
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
    for (u32 Idx = 0; Idx < GameState->ap.hand_count; ++Idx)
    {
      hand *CurrentHand = &GameState->ap.hands[Idx];
      if (CheckBust(&DealerHand->value) ||
        (!CheckBust(&CurrentHand->value) && DealerHand->value < CurrentHand->value))
      {
        PayOut(&GameState->ap.bankroll, CurrentHand->wager);
      }
      else if (CheckBust(&CurrentHand->value) || DealerHand->value > CurrentHand->value)
      {
        GameState->ap.bankroll -= CurrentHand->wager;
      }
    }

    NextPhase(&GameState->game_phase);
  }


  // RENDER
  // TODO: There are more draw calls then necessary here due to lack of sorting the units.
  // This should be looked at to either add the sorting or change the approach.
  Renderer->clear_color = V3(0.2f, 0.66f, 0.44f);

  ResetRenderer(Renderer);
  mat4 CenterTranslate = Mat4Translate((f32)Renderer->width*0.5f, (f32)Renderer->height*0.5f, 0.0f);
  {
    // Dealer's Cards
    hand Hand = GameState->dealer;
    for (u32 Index = 0; Index < Hand.card_count; ++Index)
    {
      mat4 Transform = Mat4Translate(Index*Renderer->card_width*0.5f, Index*Renderer->card_height*0.5f + 200.0f, 0.0f)*CenterTranslate;
      if (Index == 0 && (GameState->game_phase == START || GameState->game_phase == PLAYER))
      {
        PushCard(Renderer, { 2.0f, 0.0f }, Transform);
      }
      else
      {
        PushCard(Renderer, Hand.cards[Index].card_coords, Transform);
      }
    }
  }
  {
    // Player's Cards
    char TextContainer[256];
    for (u32 HCount = 0; HCount < GameState->ap.hand_count; ++HCount)
    {
      hand Hand = GameState->ap.hands[HCount];
      mat4 HandOffsetTransform = Mat4Iden();
      if (GameState->ap.hand_count > 1)
      {
        if (HCount == 0)
        {
          HandOffsetTransform = Mat4Translate(-105.0f, 0.0f, 0.0f);
        }
        else
        {
          HandOffsetTransform = Mat4Translate(105.0f, 0.0f, 0.0f);
        }
      }
      // TODO: Update the transforms to account for more than one hand
      for (u32 Index = 0; Index < Hand.card_count; ++Index)
      {
        if (Hand.cards[Index].is_dd)
        {
          mat4 Transform = HandOffsetTransform*Mat4Translate(((Index - 1)*Renderer->card_width*0.5f) + Renderer->card_height, Index*Renderer->card_height*0.5f - 200.0f, 0.0f)*
            CenterTranslate*
            Mat4RotateZ(PI32 / 2.0f);
          PushCard(Renderer, Hand.cards[Index].card_coords, Transform);
        }
        else
        {
          mat4 Transform = Mat4Translate(Index*Renderer->card_width*0.5f, Index*Renderer->card_height*0.5f - 200.0f, 0.0f)*CenterTranslate*HandOffsetTransform;
          PushCard(Renderer, Hand.cards[Index].card_coords, Transform);
        }
      }

      // Wager per hand
      mat4 Transform = CenterTranslate*
                       Mat4Translate(-45.0f, -Renderer->card_height*5.0f, 0.0f)*
                       HandOffsetTransform*
                       Mat4Scale(0.5f, 0.5f, 1.0f);
      _snprintf_s(TextContainer, sizeof(TextContainer), "$%.2f", Hand.wager);
      PushText(Renderer, StrFromCStr(TextContainer), Transform);
    }
  }
  {
    // 'Discard Tray'
    // TODO: I feel like there should be a way to reuse vertex data instead of pushing an identical card.
    if (Shoe->discarded > 0 && !FirstRound)
    {
      mat4 CardTransform = Mat4Translate((Renderer->card_width*0.5f) + Renderer->card_height,
                                     Renderer->card_height*0.5f + Renderer->width*0.25f, 1.0f)*Mat4RotateZ(PI32 / 4.0f);
      PushCard(Renderer, { 2.0f, 0.0f }, CardTransform);
      char TextContainer[256];
      // Cards in discard tray
      _snprintf_s(TextContainer, sizeof(TextContainer), "Discarded: %d", Shoe->discarded);
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
  }

}

internal void RunSimulationScene(app_state *GameState, engine_input *Input, renderer *Renderer)
{

  if (!GameState->scene_initialized)
  {
    GameState->scene_initialized = true;
  }

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
      BackToMenu(Controller, GameState);
      engine_button_state ActionDown = Controller->action_down;
      if (ActionDown.half_transitions != 0 && ActionDown.is_down)
      {
      }
    }
  }

  Renderer->clear_color = V3(0.2f, 0.44f, 0.66f);
  ResetRenderer(Renderer);

}


internal void UpdateAndRender(thread_context *Thread, app_memory *Memory, engine_input *Input, renderer *Renderer)
{
  NeoAssert(sizeof(app_state) <= Memory->perm_storage_size);
  app_state *GameState = (app_state *)Memory->perm_memory;

  if(!Memory->is_init)
  {
    InitArena(&GameState->core_arena, Memory->perm_storage_size - sizeof(app_state),
              (u8 *)Memory->perm_memory + sizeof(app_state));

    GameState->scene = s_MENU;
    Memory->is_init = true;
  }

  // TODO: Clear and reset states when scene switching is added.
  switch (GameState->scene)
  {
    case s_MENU:
    {
      RunMenuScene(GameState, Input, Renderer);
    } break;
    case s_GAME:
    {
      if (!GameState->scene_initialized)
      {
        ResetGameState(GameState, Memory, s_GAME);
      }
      RunGameScene(GameState, Input, Renderer);
    } break;
    case s_SIMU:
    {
      if (!GameState->scene_initialized)
      {
        ResetGameState(GameState, Memory, s_SIMU);
      }
      RunSimulationScene(GameState, Input, Renderer);
    } break;

    default: {};
  }
}

