#pragma once
#include "core.unity.h"

internal void EvaluateBasicStrategy(hand *PlayerHand, hand *DealerHand)
{
  rank FaceCard = DealerHand->cards[1].rank;

  if (PlayerHand->cards[0].rank == PlayerHand->cards[1].rank)
  {
    // TODO: Eval split
  }
  else if (PlayerHand->cards[0].rank == ACE || PlayerHand->cards[1].rank == ACE)
  {
    // TODO: Eval soft
  }
  else
  {
    // TODO: Eval hard
  }
}
