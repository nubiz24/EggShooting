/*********************************************************************************/
/********** THIS FILE IS GENERATED BY TOUCHGFX DESIGNER, DO NOT MODIFY ***********/
/*********************************************************************************/
#include <gui_generated/screen3_screen/Screen3ViewBase.hpp>
#include <touchgfx/Color.hpp>
#include <images/BitmapDatabase.hpp>
#include <texts/TextKeysAndLanguages.hpp>

Screen3ViewBase::Screen3ViewBase() :
    flexButtonCallback(this, &Screen3ViewBase::flexButtonCallbackHandler)
{
    __background.setPosition(0, 0, 240, 320);
    __background.setColor(touchgfx::Color::getColorFromRGB(0, 0, 0));
    add(__background);

    Back.setIconBitmaps(Bitmap(BITMAP_BACK_ID), Bitmap(BITMAP_BACK_ID));
    Back.setIconXY(0, 0);
    Back.setAction(flexButtonCallback);
    Back.setPosition(208, 288, 32, 32);
    add(Back);

    scoreArea.setXY(77, 254);
    scoreArea.setColor(touchgfx::Color::getColorFromRGB(77, 255, 0));
    scoreArea.setLinespacing(0);
    Unicode::snprintf(scoreAreaBuffer, SCOREAREA_SIZE, "%s", touchgfx::TypedText(T_SCORE).getText());
    scoreArea.setWildcard(scoreAreaBuffer);
    scoreArea.resizeToCurrentText();
    scoreArea.setTypedText(touchgfx::TypedText(T___SINGLEUSE_7ZZH));
    add(scoreArea);

    game_over1.setXY(53, 201);
    game_over1.setColor(touchgfx::Color::getColorFromRGB(0, 167, 250));
    game_over1.setLinespacing(0);
    game_over1.setTypedText(touchgfx::TypedText(T___SINGLEUSE_5MBF));
    add(game_over1);

    game_over.setXY(60, 66);
    game_over.setBitmap(touchgfx::Bitmap(BITMAP_GAME_OVER_ID));
    add(game_over);
}

Screen3ViewBase::~Screen3ViewBase()
{

}

void Screen3ViewBase::setupScreen()
{

}

void Screen3ViewBase::flexButtonCallbackHandler(const touchgfx::AbstractButtonContainer& src)
{
    if (&src == &Back)
    {
        //back
        //When Back clicked change screen to Screen1
        //Go to Screen1 with screen transition towards East
        application().gotoScreen1ScreenSlideTransitionEast();
    }
}
