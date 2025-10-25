#include "TopPage.h"

#include "Pages.h"

#include "ui/model/NoteSequenceListModel.h"
#include "ui/model/CurveSequenceListModel.h"

#include "ui/LedPainter.h"
#include "os/os.h"

TopPage::TopPage(PageManager &manager, PageContext &context) :
    BasePage(manager, context)
{
}

void TopPage::init() {
    setMode(Mode::Project);

    _context.model.project().watch([this] (Project::Event event) {
        auto &pages = _manager.pages();
        switch (event) {
        case Project::Event::ProjectCleared:
        case Project::Event::ProjectRead:
            // reset local state in pages
            pages.routing.reset();
            pages.midiOutput.reset();
            pages.song.reset();
            setMode(_mode);
            break;
        case Project::Event::TrackModeChanged:
        case Project::Event::SelectedTrackIndexChanged:
        case Project::Event::SelectedPatternIndexChanged:
            setMode(_mode);
            break;
        }
    });
}

void TopPage::editRoute(Routing::Target target, int trackIndex) {
    auto &routing = _project.routing();

    if (target == Routing::Target::None) {
        return;
    }

    int routeIndex = routing.findRoute(target, trackIndex);
    if (routeIndex >= 0) {
        setMode(Mode::Routing);
        _manager.pages().routing.showRoute(routeIndex);
        return;
    }

    routeIndex = routing.findEmptyRoute();
    if (routeIndex >= 0) {
        routing.route(routeIndex).clear();
        Routing::Route initRoute;
        initRoute.setTarget(target);
        initRoute.setTracks(1<<trackIndex);
        setMode(Mode::Routing);
        _manager.pages().routing.showRoute(routeIndex, &initRoute);
    } else {
        showMessage("All routes are used!");
    }
}

void TopPage::updateLeds(Leds &leds) {
    bool clockTick = _engine.clockRunning() && _engine.tick() % CONFIG_PPQN < (CONFIG_PPQN / 8);

    leds.set(
        Key::Play,
        _engine.recording() && !clockTick,
        clockTick
    );

    if (globalKeyState()[Key::Page] && !globalKeyState()[Key::Shift]) {
        LedPainter::drawSelectedPage(leds, _mode);
    } else {
        LedPainter::drawTrackGatesAndSelectedTrack(leds, _engine, _project.playState(), _project.selectedTrackIndex());
    }
}

void TopPage::keyDown(KeyEvent &event) {
    event.consume();
}

void TopPage::keyUp(KeyEvent &event) {
    event.consume();
}

void TopPage::keyPress(KeyPressEvent &event) {
    auto &pages = _manager.pages();
    const auto &key = event.key();

    // Page+Right: Toggle track bank
    if (key.pageModifier() && key.isRight()) {
        _trackBank = 1 - _trackBank;  // Toggle between 0 and 1
        event.consume();
        return;
    }

    if (key.isTrackSelect()) {
        int trackButton = key.trackSelect();  // Returns 0-7
        int targetTrack = (_trackBank * 8) + trackButton;

        // Double-tap detection: Toggle between track N and track N+8
        uint32_t now = os::ticks();
        uint32_t doubleTapWindow = os::time::ms(400);  // 400ms window

        if (_lastTrackPressed == trackButton &&
            (now - _lastTrackPressTime[trackButton]) < doubleTapWindow) {
            // Double tap detected - toggle to opposite bank's same button
            targetTrack = ((1 - _trackBank) * 8) + trackButton;
            _trackBank = 1 - _trackBank;  // Switch bank
            _lastTrackPressed = -1;  // Reset to avoid triple-tap
        } else {
            _lastTrackPressed = trackButton;
            _lastTrackPressTime[trackButton] = now;
        }

        _project.setSelectedTrackIndex(targetTrack);
        event.consume();
    }
    if (key.isTrack() && event.count() == 2) {
        setMode(Mode::SequenceEdit);
        event.consume();
        return;
    }

    if (key.pageModifier()) {
        setMode(Mode(key.code()));
        event.consume();
    } else {
        if (key.isPattern() && _mode != Mode::Pattern) {
            pages.pattern.setModal(true);
            pages.pattern.show();
            event.consume();
        }
        if (key.isPerformer() && _mode != Mode::Performer) {
            pages.performer.setModal(true);
            pages.performer.show();
            event.consume();
        }
    }

    if (key.isPlay()) {
        if (key.pageModifier()) {
            _engine.toggleRecording();
        } else {
            _engine.togglePlay(key.shiftModifier());
        }
        event.consume();
    }

    if (key.isTempo()) {
        if (!key.pageModifier()) {
            // tempo page
            pages.tempo.show();
        }
    }

    event.consume();
}

void TopPage::encoder(EncoderEvent &event) {
    event.consume();
}

void TopPage::setMode(Mode mode) {
    auto &pages = _manager.pages();

    _lastMode = _mode;

    switch (mode) {
    case Mode::Project:
        setMainPage(pages.project);
        break;
    case Mode::Layout:
        setMainPage(pages.layout);
        break;
    case Mode::Track:
        setMainPage(pages.track);
        break;
    case Mode::Sequence:
        setSequencePage();
        break;
    case Mode::SequenceEdit:
        setSequenceEditPage();
        break;
    case Mode::Pattern:
        pages.pattern.setModal(false);
        // do not re-enter pattern page when its already the selected page
        // the reason for this is that when changing a pattern in latched mode, we don't want to loose the latch
        // state on the page
        if (_manager.top() != &pages.pattern) {
            setMainPage(pages.pattern);
        }
        break;
    case Mode::Performer:
        pages.performer.setModal(false);
        setMainPage(pages.performer);
        break;
    case Mode::Overview:
        setMainPage(pages.overview);
        break;
    case Mode::Clock:
        setMainPage(pages.clockSetup);
        break;
    case Mode::Song:
        setMainPage(pages.song);
        break;
    case Mode::Routing:
        setMainPage(pages.routing);
        break;
    case Mode::MidiOutput:
        setMainPage(pages.midiOutput);
        break;
    case Mode::UserScale:
        setMainPage(pages.userScale);
        break;
    case Mode::Modulator:
        setMainPage(pages.modulator);
        break;
    case Mode::Monitor:
        setMainPage(pages.monitor);
        break;
    case Mode::System:
        if (mode != _lastMode) {
            _manager.pages().confirmation.show("DO YOU REALLY WANT TO ENTER SYSTEM PAGE?", [this] (bool result) {
                if (result) {
                    setMainPage(_manager.pages().system);
                } else {
                    setMode(_lastMode);
                }
            });
        }
        break;
    default:
        return;
    }

    _mode = mode;
}

void TopPage::setMainPage(Page &page) {
    if (_manager.stackSize() < 2) {
        _manager.push(&page);
    } else {
        _manager.replace(1, &page);
    }
}

void TopPage::setSequencePage() {
    auto &pages = _manager.pages();

    switch (_project.selectedTrack().trackMode()) {
    case Track::TrackMode::Note:
        setMainPage(pages.noteSequence);
        break;
#if CONFIG_ENABLE_CURVE_TRACKS
    case Track::TrackMode::Curve:
        setMainPage(pages.curveSequence);
        break;
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
    case Track::TrackMode::MidiCv:
        setMainPage(pages.track);
        break;
#endif
    case Track::TrackMode::Last:
        break;
    }
}

void TopPage::setSequenceEditPage() {
    auto &pages = _manager.pages();

    switch (_project.selectedTrack().trackMode()) {
    case Track::TrackMode::Note:
        setMainPage(pages.noteSequenceEdit);
        break;
#if CONFIG_ENABLE_CURVE_TRACKS
    case Track::TrackMode::Curve:
        setMainPage(pages.curveSequenceEdit);
        break;
#endif
#if CONFIG_ENABLE_MIDICV_TRACKS
    case Track::TrackMode::MidiCv:
        setMainPage(pages.track);
        break;
#endif
    case Track::TrackMode::Last:
        break;
    }
}
