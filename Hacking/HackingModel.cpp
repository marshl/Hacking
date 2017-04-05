#include "HackingModel.h"

#include <list>
#include <sstream>
#include <cassert>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "strfunc.h"
#include "PuzzleWord.h"
#include "DifficultyLevel.h"

HackingModel::HackingModel()
{
    this->currentHighlightedPuzzleWord = nullptr;
    this->currentDifficulty = nullptr;

    this->InitialiseDifficultyLevels();

    const std::string FILLER_CHARACTERS = "$-/?|=+&^%!@#&*{}[]()<>,.'\"";
    const int FILLER_CHARACTER_COUNT = FILLER_CHARACTERS.length();

    for ( int i = 0; i < this->GetTotalLineCount(); ++i )
    {
        std::string filler( '#', this->GetColumnWidth() );
        for ( int j = 0; j < this->GetColumnWidth(); ++j )
        {
            filler[i] = FILLER_CHARACTERS[rand() % FILLER_CHARACTER_COUNT];
        }

        this->fillerCharacters.push_back( filler );
    }
}

HackingModel::~HackingModel()
{
    for ( unsigned int i = 0; i < this->puzzleWords.size(); ++i )
    {
        delete this->puzzleWords[i];
    }

    for ( unsigned int i = 0; i < this->difficultyLevels.size(); ++i )
    {
        delete this->difficultyLevels[i];
    }
}

int HackingModel::GetAttemptsRemaining() const
{
    return this->attemptsRemaining;
}

const int HackingModel::GetPuzzleWordCount() const
{
    return this->puzzleWords.size();
}

const int HackingModel::GetAttemptedWordCount() const
{
    return this->attemptedWords.size();
}

PuzzleWord * const HackingModel::GetPuzzleWord( int index ) const
{
    return this->puzzleWords[index];
}

PuzzleWord * const HackingModel::GetSelectedPuzzleWord() const
{
    return this->currentHighlightedPuzzleWord;
}

void HackingModel::OnClickEvent( const COORD& cursorCoord )
{
    if ( this->currentHighlightedPuzzleWord != nullptr )
    {
        int diff = StringDiff( this->currentHighlightedPuzzleWord->GetText(), this->solutionWord );

        if ( diff == 0 )
        {

        }
        else
        {
            this->attemptedWords.push_back( this->currentHighlightedPuzzleWord->GetText() );
            --this->attemptsRemaining;
        }
    }
}

void HackingModel::OnMouseMoveEvent( const COORD& cursorCoord )
{
    if ( this->currentDifficulty == nullptr )
    {

    }
    else
    {
        this->currentHighlightedPuzzleWord = nullptr;

        for ( unsigned int i = 0; i < puzzleWords.size(); ++i )
        {
            this->puzzleWords[i]->SetHighlight( false );
        }

        for ( unsigned int i = 0; i < this->puzzleWords.size() && this->currentHighlightedPuzzleWord == nullptr; ++i )
        {
            PuzzleWord* word = this->puzzleWords[i];

            for ( int j = 0; j < this->GetCurrentDifficulty()->GetWordLength(); ++j )
            {
                if ( word->GetScreenCoord( j ).X == cursorCoord.X
                    && word->GetScreenCoord( j ).Y == cursorCoord.Y )
                {
                    word->SetHighlight( true );
                    this->currentHighlightedPuzzleWord = word;
                    break;
                }
            }
        }
    }
}

const DifficultyLevel * const HackingModel::GetCurrentDifficulty() const
{
    return this->currentDifficulty;
}

const DifficultyLevel * const HackingModel::GetDifficultyLevelWithIndex( int index ) const
{
    return this->difficultyLevels[index];
}

int HackingModel::GetDifficultyCount() const
{
    return this->difficultyLevels.size();
}

void HackingModel::SetDifficultyLevel( int index )
{
    assert( index >= 0 && index < this->GetDifficultyCount() );
    this->currentDifficulty = this->difficultyLevels[index];
}

const int HackingModel::GetColumnWidth() const
{
    return 12;
}

const int HackingModel::GetColumnHeight() const
{
    return 17;
}

const int HackingModel::GetColumnCount() const
{
    return 2;
}

const int HackingModel::GetTotalLineCount() const
{
    return this->GetColumnHeight() * this->GetColumnCount();
}

const int HackingModel::GetTotalColumnCharacterCount() const
{
    return this->GetColumnCount() * this->GetColumnWidth() * this->GetColumnHeight();;
}

const int HackingModel::GetMaximumWordLength() const
{
    int maximumLength = 0;
    for ( int i = 0; i < ( int )this->difficultyLevels.size(); ++i )
    {
        if ( this->GetDifficultyLevelWithIndex( i )->GetWordLength() > maximumLength )
        {
            maximumLength = this->GetDifficultyLevelWithIndex( i )->GetWordLength();
        }
    }

    return maximumLength;
}



void HackingModel::SetPuzzleWords()
{
    std::vector<std::string> allWords;
    std::vector<std::string> solutionWords;
    std::vector<std::string> triedWords;

    this->GetSampleWordList( allWords );

    assert( (int)allWords.size() >= this->GetCurrentDifficulty()->GetWordCount() );

    std::random_shuffle( allWords.begin(), allWords.end() );

    while ( true )
    {
        std::string randWord = allWords.back();
        bool wordAdded = false;

        // Add the word if there are no others
        if ( solutionWords.size() == 0 )
        {
            solutionWords.push_back( randWord );
            wordAdded = true;
        }
        else
        {
            for each( const std::string& solutionWord in solutionWords )
            {
                int difference = StringDiff( solutionWord, randWord );
                if ( difference > this->GetCurrentDifficulty()->GetLetterDifference() || difference == 0 )
                {
                    continue;
                }

                solutionWords.push_back( randWord );
                wordAdded = true;

                if ( solutionWords.size() == this->GetCurrentDifficulty()->GetWordCount() )
                {
                    for each( std::string word in solutionWords )
                    {
                        this->puzzleWords.push_back( new PuzzleWord( word ) );
                    }
                    return;
                }

                // More words are needed, conitnue the loop
                break;
            }
        }

        triedWords.push_back( randWord );
        allWords.pop_back();

        if ( allWords.size() == 0 )
        {
            allWords.insert( allWords.begin(), triedWords.begin(), triedWords.end() );
            triedWords.clear();
            solutionWords.clear();
            std::random_shuffle( allWords.begin(), allWords.end() );
        }
    }
}

void HackingModel::PlacePuzzleWords()
{
    assert( this->GetTotalColumnCharacterCount() > this->GetCurrentDifficulty()->GetWordLength() );

    for ( unsigned int i = 0; i < puzzleWords.size(); )
    {
        // Theoretically, this might go infinite with the right conditions
        int place = rand() % ( this->GetTotalColumnCharacterCount() - this->GetCurrentDifficulty()->GetWordLength() );
        bool badPlacement = false;
        // Check for position collisions with words that have already been placed
        for ( unsigned int j = 0; j < i; ++j )
        {
            if ( std::abs( puzzleWords[j]->GetPosition() - place ) < this->GetCurrentDifficulty()->GetWordLength() + 1 )
            {
                badPlacement = true;
                break;
            }
        }

        if ( !badPlacement )
        {
            puzzleWords[i]->SetPosition( place );
            ++i;
        }
    }

    for ( unsigned int i = 0; i < puzzleWords.size(); ++i )
    {
        int place = puzzleWords[i]->GetPosition();

        int x = place % this->GetColumnWidth();
        int y = ( place - x ) / this->GetColumnWidth();

        int column = 0;

        while ( y >= this->GetColumnHeight() )
        {
            ++column;
            y -= this->GetColumnHeight();
        }

        // If the std::string goes over the end of a line
        if ( x + this->GetCurrentDifficulty()->GetWordLength() > this->GetColumnWidth() )
        {
            // Separate it into two
            std::string chunk1 = puzzleWords[i]->GetText().substr( 0, this->GetColumnWidth() - x + 1 );
            std::string chunk2 = puzzleWords[i]->GetText().substr( this->GetColumnWidth() - x + 1 );

            for ( int j = 0; j < this->GetColumnWidth() - x + 1; ++j )
            {
                COORD& coord = puzzleWords[i]->GetScreenCoord( j );

                coord.X = x;// +HEX_CODE_LENGTH + column * TOTAL_COLUMN_WIDTH + j;
                // TODO: Offset this in the view/controller
                coord.Y = y;// +LINES_BEFORE_COLUMNS;
            }

            // Render one half
            ++y;
            if ( y >= this->GetColumnHeight() )
            {
                ++column;
                y -= this->GetColumnHeight();
            }

            int k = 0;
            for ( int j = this->GetColumnWidth() - x + 1; j < this->GetCurrentDifficulty()->GetWordLength(); ++j )
            {
                COORD& coord = puzzleWords[i]->GetScreenCoord( j );
                coord.X = this->GetColumnWidth() + 1 + column * this->GetColumnWidth() + k;
                // TODO: Offset this in the view/controller
                coord.Y = y;// + LINES_BEFORE_COLUMNS;
                ++k;
            }
        }
        else
        {
            for ( int j = 0; j < this->GetCurrentDifficulty()->GetWordLength(); ++j )
            {
                COORD& coord = puzzleWords[i]->GetScreenCoord( j );
                coord.X = x;// +1 + HEX_CODE_LENGTH + column * TOTAL_COLUMN_WIDTH + j;
                coord.Y = y;// + LINES_BEFORE_COLUMNS;
            }
        }
    }
}

void HackingModel::GetSampleWordList( std::vector<std::string>& _out )
{
    std::ifstream fin( "dictionary" );
    if ( !fin.is_open() )
    {
        std::cout << "Error opening dictionary\n";
        exit( 1 );
    }

    std::string word;
    while ( fin >> word )
    {
        if ( word.length() == this->GetCurrentDifficulty()->GetWordLength() )
        {
            std::transform( word.begin(), word.end(), word.begin(), ::toupper );
            _out.push_back( word );
        }
    }

    fin.close();
}

void HackingModel::InitialiseDifficultyLevels()
{
    this->difficultyLevels.push_back( new DifficultyLevel( 0, "Very Easy", 9, 5, 2, 4 ) );
    this->difficultyLevels.push_back( new DifficultyLevel( 1, "Easy", 9, 7, 3, 4 ) );
    this->difficultyLevels.push_back( new DifficultyLevel( 2, "Average", 8, 9, 3, 4 ) );
    this->difficultyLevels.push_back( new DifficultyLevel( 3, "Hard", 7, 11, 4, 4 ) );
    this->difficultyLevels.push_back( new DifficultyLevel( 4, "Very Hard", 7, 13, 4, 4 ) );
}