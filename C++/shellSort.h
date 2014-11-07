// Shellsort Algorithm



 
void shellSort(int sortedArray[]) {
	// Valid gap sizes are according to AT&T research
	static const int gaps[] = {1, 4, 10, 23, 57, 132, 301, 701, 1612, 3708};
	// ^ = Valid, ? = Guess    ^  ^  ^^  ^^  ^^  ^^^  ^^^  ^^^  ????  ????
	
	// jGap is an index for the gap array
	for (int jGap = gaps.size() - 1; jGap >= 0; --jGap) {		
		// Insertion sort with gap between sorted elements
		for (int currentGap = gaps[jGap]; currentGap < sortedArray.size(); ++currentGap) {
			
			// Store current value
			int insVal = sortedArray[currentGap];
			
			// Search for correct place to insert current value
			int j;
			for (j = currentGap - gaps[jGap]; j >= 0 && sortedArray[j] > insVal; j -= gaps[jGap]) {
				// Swap values up the list to make room for current value
				sortedArray[j + gaps[jGap]] = sortedArray[j];
			}
			// When appropriate place is found, insert current value
			sortedArray[j + gaps[jGap]] = insVal;
		}
	}
	
}