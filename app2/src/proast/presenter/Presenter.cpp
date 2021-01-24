#include <proast/presenter/Presenter.hpp>

namespace proast { namespace presenter { 
    Presenter::Presenter(model::Model &model, view::View &view): model_(model), view_(view)
    {
        view_.events = this;
    }

    void Presenter::run()
    {
        view_.header = L"ok";
        n00_ = dto::List::create();
        n00_->items.emplace_back(L"zero");
        n00_->items.emplace_back(L"one");
        n00_->items.emplace_back(L"two");
        n00_->items.emplace_back(L"three");
        n00_->items.emplace_back(L"four");
        n00_->items.emplace_back(L"five");
        n00_->items.emplace_back(L"six");
        n00_->items.emplace_back(L"seven");
        view_.n00 = n00_;
        view_.run();
    }

    //Commander API
    void Presenter::commander_quit()
    {
        view_.quit();
    }
    void Presenter::commander_move(Direction direction)
    {
        oss_.str(L"");
        switch (direction)
        {
            case Direction::Down:  
                oss_ << L"Down"; 
                ++n00_->ix;
                break;
            case Direction::Up:    
                oss_ << L"Up"; 
                --n00_->ix;
                break;
            case Direction::Left:  
                oss_ << L"Left"; 
                break;
            case Direction::Right: 
                oss_ << L"Right"; 
                break;
        }
        view_.footer = oss_.str();
    }

    //View::Events API
    void Presenter::received(wchar_t wchar)
    {
        Commander::process(wchar);
    }
} } 
