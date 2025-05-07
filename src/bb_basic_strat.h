enum Action
{
  NONE = 0,
  SPLITNO = 'N',
  SPLITYES = 'Y',
  HIT = 'H',
  STAND = 'S',
  DD = 'D',
};

// NOTE: Default will be S17 (US)
// TODO: Handle Late surrenders

// Dealer Cards horizontal; Player cards vertical
char SPLITS[] =
{
  // 2   3   4   5   6   7   8   9   10  A
    'Y','Y','Y','Y','Y','Y','Y','Y','Y','Y', // A, A
    'N','N','N','N','N','N','N','N','N','N', // T, T
    'Y','Y','Y','Y','Y','N','Y','Y','N','N', // 9, 9
    'Y','Y','Y','Y','Y','Y','Y','Y','Y','Y', // 8, 8
    'Y','Y','Y','Y','Y','Y','N','N','N','N', // 7, 7
    'Y','Y','Y','Y','Y','N','N','N','N','N', // 6, 6
    'N','N','N','N','N','N','N','N','N','N', // 5, 5
    'N','N','N','Y','Y','N','N','N','N','Y', // 4, 4
    'Y','Y','Y','Y','Y','Y','N','N','N','N', // 3, 3
    'Y','Y','Y','Y','Y','Y','N','N','N','N', // 2, 2
};

char SOFTS[] =
{
  // 2   3   4   5   6   7   8   9   10  A
    'S','S','S','S','S','S','S','S','S','S', // A, 9
    'S','S','S','S','S','S','S','S','S','S', // A, 8
    'Y','D','D','D','D','S','S','H','H','H', // A, 7
    'H','D','D','D','D','H','H','H','H','H', // A, 6
    'H','H','D','D','D','H','H','H','H','H', // A, 5
    'H','H','D','D','D','H','H','H','H','H', // A, 4
    'H','H','H','D','D','H','H','H','H','H', // A, 3
    'H','H','H','D','D','H','H','H','H','H', // A, 2
};

char HARDS[] =
{
  // 2   3   4   5   6   7   8   9   10  A
    'S','S','S','S','S','S','S','S','S','S', // 17
    'S','S','S','S','S','H','H','H','H','H', // 16
    'S','S','S','S','S','H','H','H','H','H', // 15
    'S','S','S','S','S','H','H','H','H','H', // 14
    'S','S','S','S','S','H','H','H','H','H', // 13
    'H','H','S','S','S','H','H','H','H','H', // 12
    'D','D','D','D','D','D','D','D','D','H', // 11
    'D','D','D','D','D','D','D','D','H','H', // 10
    'H','D','D','D','D','H','H','H','H','H', // 9
    'H','H','H','H','H','H','H','H','H','H', // 8
};

