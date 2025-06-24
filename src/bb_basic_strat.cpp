#pragma once
#include "core.unity.h"

internal const u32 TableStride = 10;

internal void UpdateCharts(table_rules TableRules)
{
  if (TableRules.h17)
  {
    // A, 8 vs a 6
    SOFTS[14] = 'D';
    // A, 7 vs a 2
    SOFTS[20] = 'D';
    // 11 vs a A
    HARDS[69] = 'D';
  }
}
// internal void PlayCorrectAction(char CorrectAction)
// {
//   switch (CorrectAction)
//   {
//     case HIT:
//     {
//       Hit()
//     } break;
//     case STAND:
//     {
//     } break;
//     case DD:
//     {
//     } break;
//     case SPLITYES:
//     {
//     } break;
//     case SPLITNO:
//     {
//     } break;
//   }
// }

internal void EvaluateBasicStrategy(hand *PlayerHand, hand *DealerHand)
{
  rank FaceCard = DealerHand->cards[1].rank;

  if (PlayerHand->cards[0].rank == PlayerHand->cards[1].rank)
  {
    ums Idx = FaceCard + (ACE - PlayerHand->cards[0].rank)*TableStride;
    char CorrectAction = SPLITS[Idx];
  }
  else if (PlayerHand->cards[0].rank == ACE || PlayerHand->cards[1].rank == ACE)
  {
    rank NonAceCard = PlayerHand->cards[0].rank == ACE ?
                      PlayerHand->cards[0].rank :
                      PlayerHand->cards[1].rank;
    ums Idx = FaceCard + (ACE - 2 - NonAceCard)*TableStride;
    char CorrectAction = SOFTS[Idx];
  }
  else
  {
    char CorrectAction = 0;
    if (PlayerHand->value >= 18)
      CorrectAction = STAND;
    else
    {
      ums Idx = FaceCard + (17 - PlayerHand->value)*TableStride;
      CorrectAction = HARDS[Idx];
    }
  }
}
