# Development Notes

# dev-notes.md

## UI / Layout tweaks  (2025-10-27)

- **Step-edit Generator mode:** buttons should toggle active/inactive so a single tap + encoder turn adjusts value.
- **Font glyph alignment:** plus sign and full-stop sit too high versus numbers; centre them vertically.
- **Step-edit note/octave rows:** two rows are 1 px too close; shift octave row down 1 px.
- **Step-edit no-step-selected:** encoder should scroll parameters inside current function (gate, ratchet, length, note, condition).
- **Pattern view:** pattern labels P1/P2/… under pattern boxes need +3 px vertical offset.
- **Song mode:** all main content (header/footer excluded) raise 2 px.
- **Two-column lists:** make column widths dynamic to fit content; remove fixed large gaps, prevent long-word crowding.
- **Routing page track-select boxes:** row sits almost a full row too low; pull back up.
- **Routing page:** confirm if boxes are drawn shapes or font glyphs (may share glyph-offset bug).
- **Header bar:** add a few pixels between BPM and track number (shift track-number group right).

## Keyboard-page prototype issues

- **Key width:** keys drawn too narrow; screen is wide enough for full two octaves at original size—revert width and shrink only if needed.
- **Black-key pattern:** currently broken (extra white keys, missing blacks); must follow correct piano layout (W-B-W-B-W-W-B-W-B-W-B-W-W).
- **White-key LEDs:** should be solid green but are all off.
- **LED scrolling:** moving focus scrambles black-key LED indicators.
- **Root-note text:** too small, should be bold like track number.
- **Pagination/margin:** accidental glyph (♯/♭) overwrites root note; shift root-note + track-number group left to fix.
- **Missing octave indicator:** add visible octave number for root note.

07:53

# firmware-fixes.md

## Modulated-channel visual feedback
- Add slim vertical bar (same width/look as scroll-bar) right-hand side of waveform window, same height as window, slightly offset inset
- Fill bar with white; draw bold white line inside that moves vertically to mirror real-time output signal (0–127 MIDI range) for instant level feedback

## Euclidean rhythm generator
- Make function buttons toggles (stay active) so single tap + encoder turn sets parameter
- &gt;16 steps: auto-add second row of step indicators to keep individual step boxes from becoming too narrow / hard to see
- Enlarge step boxes to same size/style as used in step-edit mode for consistency & visibility

## Keyboard page
- Place EXIT button in exact horizontal centre
- Keep F1 / F2 left side for semitone shifts, right side for octave shifts (current layout OK)
- Widen keys a little more
- Remove the “extra” white note that appears between every real note; enforce correct W/B count
- Enable focus-window to lock onto every semitone (black & white) while scrolling
- Move keyboard-edge LED indicators tight to the keys; stop them overlapping other UI elements
- Centre the track-number + root-note block horizontally on the keyboard page
- Centre pagination dots vertically on the keyboard vertical mid-line

## Perform All Page
- Pattern numbers have been raised up by a few pixels this is not good they were fine before.

## Perform Page
- Perform Page Bank 1 and 2 for T1-T8 and T9-T16 the pattern number seems to have been shifted in the wrong direction. they need to be shifted up by double the amount they were wrongly offset towards the bottom of the screen

## Font Issues
- All the Symbols like . + - are still not properly aligned it seems nothing has changed with this.
- the note and octave in note edit need some space between them. 1 pxl should suffice.
- same thing for the COND funtions when there is two rows of information.

## Startup Screen
- The text can be change to WESTLICHT ❤️MODULOVE❤️PHASERVILLE
- The space invaders can be bigger and bounce out of sync with each other.
I have updated the drawn SVG logo which you rendered from the code of the drawn logo for the boot screen. on the desktop.
- The background of the loading screen should have a hyperspace style vortex animation that is moving forwards like we are travelling through a tunnel. this should be dark and flashing randomly with white streaks to resemble hyperspeed.

## Route Page
- the track boxes have been shifted almost perfectly but need to move down 3 pxl and then back possibly need to see how the dynamic collumns look first.

## Commit Preperations
-I need a fully updated changelog with all the changes that have been implemented. analyse the differences in the code base with the original and create a detailed changelog ready to commit to git hub.
create a flow chart for you to reference with all the dependencies and high and low level functions. check for things that have been called twice in different ways to see if we can optimise some thing by adding them to a recallable function.