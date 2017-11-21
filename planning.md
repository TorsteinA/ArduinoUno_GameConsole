# Read/write planning:

## Fields

Field with int[] for scores.


## Write    (Save)

* Open file
* Write new score to file on new line
* close file

## Read     (Load)

* Open file
* Load values from file to tempArray
* Close file
* return tempArray


## Update high scores

* Load all values from file into tempArray
* sort tempArray in decreasing order
* fill highScoreOne array with elements from tempArray until highScoreOne array is filled
* clear file
* fill file with highScoreOne array data
* Implement for game2 after game2 is implemented

This will make sure the top X high scores are saved on file, where X is the size of the highScoreOne array, which is also the number of high scores to show in the high score menu



