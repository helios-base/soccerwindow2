# Exporting images and videos

soccerwindow2 saves what the field view shows as images, and exports ranges of a game log as images or videos, drawn as they look on the screen.
The same export runs from the command line, without a window.
It is ported from rcssmonitor: the dialog, the options and the names of the files are the same.

- [What you need](#what-you-need)
- [Saving the frame shown](#saving-the-frame-shown)
- [The export dialog](#the-export-dialog)
  - [Ranges](#ranges)
  - [Marking a range in the main window](#marking-a-range-in-the-main-window)
  - [Presets](#presets)
  - [Images: the size and the rate](#images-the-size-and-the-rate)
  - [Output: images or a video](#output-images-or-a-video)
  - [Exporting](#exporting)
- [What is drawn](#what-is-drawn)
- [Rates, speeds and lengths](#rates-speeds-and-lengths)
- [The command line](#the-command-line)
  - [Options](#options)
  - [Ranges on the command line](#ranges-on-the-command-line)
  - [Examples](#examples)
  - [Without a display](#without-a-display)
  - [Instead of --auto-image-save](#instead-of---auto-image-save)
- [Sizes of the files](#sizes-of-the-files)
- [Tips](#tips)
  - [Still frames at the start and the end](#still-frames-at-the-start-and-the-end)
  - [Pauses between the scenes](#pauses-between-the-scenes)
  - [One compression from the images](#one-compression-from-the-images)
  - [Fading from and to black](#fading-from-and-to-black)
- [When something goes wrong](#when-something-goes-wrong)

## What you need

- Images: nothing more.
- Videos: [ffmpeg](https://ffmpeg.org/), installed separately (`sudo apt install ffmpeg` on Ubuntu), with libx264 for H.264 (most builds have it). soccerwindow2 runs it as a program: in the PATH, or where you choose. soccerwindow2 is built without it.
- Ranges are exported from a game log, not while connected to a server: the frames kept after disconnecting can be exported.

## Saving the frame shown

`File` -> `Save Image of This Frame...` (`Ctrl+Shift+I`) saves the frame shown to a PNG file (or JPEG by the extension `.jpg` or `.jpeg`; PNG when there is no extension).
It can be used while connected to a server too.

- The image is as large as the field view: on a high-DPI screen, with as many pixels as the screen shows.
- The default name is the game log's name and the cycle, e.g. `202307090956-Oxsy_2-vs-YuShan2023_2-1957.png` (`-1957.3` for a stopped frame). For the frames received from a server and in the anonymous mode, it is `soccerwindow2-1957.png`.
- The image is drawn when the item is chosen, before the file dialog: the log may go on playing behind the dialog.
- The texts are drawn with grayscale antialiasing: they may look a little lighter than on the screen, which draws them without antialiasing.

## The export dialog

`File` -> `Export Images / Video` -> `Open the Export Dialog...` (`Ctrl+Shift+E`) opens the dialog.
It stays open while you move through the game log, so that you can take the positions shown.
It and the items of the range are enabled when there are frames (a game log open, or the frames of a live game kept after disconnecting) and soccerwindow2 is not connected to a server; while connected, `Export` is disabled.

The dialog has three parts, `Ranges`, `Images` and `Output`, and a summary of what will be exported above the `Export` button, e.g.:

```
604 images of 604 frames, in 4 ranges. 1280 x 720 pixels; 4 videos, 60.4 s in all at 10 images/s.
```

A mistake (a position not in the log, ffmpeg not found, ...) is shown there in red instead, and `Export` is disabled.

The fields are kept in the settings (the group `Export` of `~/.soccerwindow2`) when an export starts.
`--export-image-prefix` and `--export-image-format` given on the command line without an export ([Options](#options)) are used instead of the saved names and format for that run.

### Ranges

The list shows the ranges to export, e.g. `goal1-c1957: 1857 - 1957.50 (151 frames)`. Under it, the range chosen in the list:

- `Name:` its name: the name of its directory or its video when the ranges are exported each in its own.
- `Start:` and `End:` its first and last frames:
  - `1234`: the cycle 1234. As the start, its first frame; as the end, its last frame, the frames stopped in it too.
  - `1234.3`: the frame stopped in the cycle 1234 with the stopped count 3 (the score board shows it `1234,3`, which can be written too). The frames stopped in a cycle (e.g. after a goal, before the kick-off) are counted 1, 2, ...
  - `1234.0`: the first frame of the cycle 1234, before the frames stopped in it. `Now`, `]` and the presets write an end there so.
  - empty: the first frame (`Start`) or the last frame (`End`) of the log.
- Beside each, the frame and the game time it is, e.g. `1857 (3:05)`. A position with no frame (out of the log, or a cycle missing in it) is moved to the next frame for a start and to the previous one for an end, and told: `: no frame at 9000`.
- `Now` takes the position shown in the main window.
- `Clear` empties both: the whole log.
- `Presets` gives ranges made for you ([Presets](#presets)).

`Add` adds a range of the whole log at the end of the list (`range2`, `range3`, ...), `Remove` removes the range chosen, and `Up` and `Down` move it.
Several ranges are exported in one go: each in its own directory or video, or all in one sequence of images or one video (`Ranges:` in `Output`).

The ranges belong to the game log: they are forgotten when another game log is opened or soccerwindow2 connects to a server.

### Marking a range in the main window

The range chosen can also be marked in the main window, with the dialog closed:

| Key | Menu (`File` -> `Export Images / Video`) | |
|---|---|---|
| `[` | `Start the Range Here` | the position shown as the start |
| `]` | `End the Range Here` | the position shown as the end |
| | `Clear the Range` | the whole log |

The slider of the log player shows the ranges as bands, the one chosen darker, and the goals as orange ticks, also when the tool bar is at the left or the right of the window.

### Presets

`Presets` gives the range chosen (or the list) of:

- `The whole log`.
- A part of the game: `First half`, `Second half`, `Extra time 1`, `Extra time 2`, `Penalty shootout`, with their game times, e.g. `First half (0:00 - 4:59)`. The halves are found by the cycles of the server's parameters in the game log (rcssserver records no play mode at the end of a half).
- `Goal N: TIME TEAM (SCORE)`, e.g. `Goal 1: 3:15 YuShan2023 (0 - 1)`: the scene of a goal, from 10 seconds before it to 5 seconds after it. The frames stopped after a goal are 5 seconds, so the scene ends before the kick-off, e.g. `goal1-c1957: 1857 - 1957.50 (151 frames)`. The goals are the changes of the score that `Go to the next score` of the log player uses.
- `All the goals, a range each`: a range for each goal (`goal1-c1957`, `goal2-c3466`, ...), in place of the whole log, or after the ranges already in the list.
- `Around the position shown`: from 5 seconds before the frame shown to 5 seconds after it (`around-c1234`).

The seconds are set in the dialog: `Presets: Goal scenes: 10.0 s before, 5.0 s after. Around now: 5.0 s before, 5.0 s after.`
They are the seconds of the playback, as in a video at x 1: a frame is a step (100 ms), a stopped frame too.

### Images: the size and the rate

- `Size:` `As the view` (the size of the field view, in its pixels on a high-DPI screen), `1280 x 720`, `1920 x 1080`, `3840 x 2160`, or `Custom` (16 x 16 to 7680 x 4320). A video has even sizes: an odd one is made one smaller.
- `Enlarge the texts, logos and lines with the image`: the image looks as the field view, enlarged (or made smaller) to the size of the image. The texts, the team logos and the widths of the lines are as large in the image as in the view. Off, they keep their sizes in pixels: smaller and thinner in a large image.
- `Rate:` the images a second of the video (10 by default), and `Speed:` how many times as fast as the game is played. `As the Playback` sets the speed at which the log player plays the game log (the simulation step / the interval of the playback). See [Rates, speeds and lengths](#rates-speeds-and-lengths).

### Output: images or a video

`Images`:

- `Directory:` where the images are written (made if it is not there). Empty: the home directory.
- `Names:` the start of the names and the format (`PNG` or `JPG`). The images are numbered from 0, `image-00000.png`, with 5 digits or more as needed: a tool such as ffmpeg reads them as a sequence.
- `Name the images by the cycles`: `image-c01234.00.png` (the cycle and the stopped count), when each image is a frame (10 images/s at x 1).

`Video (ffmpeg)`:

- `File:` the video. Its extension gives the format: `.mp4`, `.mov` or `.mkv` (`.mp4` is added when there is none). Empty: `soccerwindow2.mp4` in the home directory.
- `Quality (CRF):` 18 by default. Smaller is better and larger; 18 shows hardly any loss, 23 is ffmpeg's default.
- `Encoding speed:` `medium` by default. Slower makes smaller files of the same quality, and takes longer.
- `ffmpeg:` its path, empty for `ffmpeg` in the PATH. The line under it shows its version and the H.264 encoder it has, e.g. `ffmpeg version 6.1.1; H.264 by libx264.`. Without libx264, another encoder is used and told (MPEG-4 Part 2 by mpeg4, larger and of lower quality); `Quality` and `Encoding speed` are then not used.
- The video is H.264 in yuv420p (the colors of 2 x 2 pixels, as most players expect), without sound, tuned for the flat colors of the field view.

`Ranges:` with several ranges, `Each in its own directory, by its name` (`DIRECTORY/NAME/image-00000.png`) or `A video for each, by its name` (`FILE-NAME.mp4`), or `All in one sequence` / `All in one video` (the numbers going on over the ranges).

### Exporting

`Export` starts the export:

1. The playback stops: the frame shown stays.
2. The files already there are told once: overwrite them, choose another directory or file, or cancel. For images, the disk space is estimated, and asked about when it may not be enough.
3. A progress dialog shows the images exported and the time left. `Stop` stops it: the images written are kept; a video being written is removed (one not finished may not play), and the videos of the ranges finished are kept.
4. At the end, the dialog and the status bar tell what was exported, e.g. `Exported 4 videos in /home/me/videos.`

The field view does not change while exporting.

## What is drawn

The images are drawn by the field view, as it draws on the screen, with the options of the `View` menu (the score board, the player numbers, the paint style, ...) and the colors and the fonts of the settings.

- The debug drawings that belong to a time are drawn for each image: the view of the debug server, the draw data (`File` -> `Open Draw Data`), the traces of the ball and the players, and the other drawings of the field view.
- The debug log of an agent (the `Debug Message` window) is drawn only in the frame synchronized with it, as on the screen: it does not follow the images of the export.
- The formation editor is not changed by an export.
- The command line draws the game log with the options given ([below](#the-command-line)): the debug server, the draw data and the debug logs are opened only by a window.

## Rates, speeds and lengths

An image advances `speed × 1000 / ( step × fps )` frames, where `step` is the simulation step (100 ms) and `fps` the images a second.
The images of a range are at its first frame and every that many frames after it, up to its last frame; each image is the frame at or before its position.

| Images/s | Speed | Frames an image | 1 minute of the game (600 frames) |
|---|---|---|---|
| 10 | x 1 | 1 | 600 images, a video of 60 s |
| 10 | x 2 | 2 (every other frame) | 300 images, 30 s |
| 20 | x 2 | 1 | 600 images, 30 s |
| 10 | x 0.5 | 0.5 (each frame twice) | 1200 images, 120 s |

Several ranges in one video last as long as all of them.

## The command line

The export runs from the command line without a window, and quits:

```bash
soccerwindow2 --export-video FILE [OPTIONS] GAME_LOG
soccerwindow2 --export-images DIR [OPTIONS] GAME_LOG
```

It reads the game log, exports it as the dialog does, tells the progress on the standard error, and quits with the exit code 0 when it is done, or 1 with the reason (`soccerwindow2: ...`) when it cannot be done.

- It draws with the colors and the fonts of the settings (`~/.soccerwindow2`, read only) and the options on the command line (e.g. `--paint-style rcssmonitor`, `--anonymous-mode`), and writes nothing in `~/.soccerwindow2`.
- The defaults are fixed, not the fields of the dialog: a script gives the same result whatever was last done in the dialog.
- The files there are overwritten without asking.

### Options

| Option | Default | |
|---|---|---|
| `--export-video FILE` | | a video by ffmpeg: `.mp4`, `.mov` or `.mkv` (`.mp4` when there is no extension) |
| `--export-images DIR` | | images in DIR |
| `--export-range RANGE` | `all` | a range ([below](#ranges-on-the-command-line)); given again for more |
| `--export-join` | off | the ranges in one video (or one sequence of images), not each in its own |
| `--export-fps N` | 10 | the images a second |
| `--export-speed N` | 1 | how many times as fast as the game is played |
| `--export-size WxH` | `1280x720` | the size of the images (even for a video) |
| `--export-scale R` | the height / 720 | the images look as a view of their size / R, enlarged R times (the texts, the logos, the lines): by default, as a view 720 pixels high |
| `--export-image-format png\|jpg` | `png` | |
| `--export-image-prefix STR` | `image-` | the start of the names of the images |
| `--export-crf N` | 18 | the quality of the video, 0 to 51 |
| `--export-preset STR` | `medium` | the encoding speed: `ultrafast`, `superfast`, `veryfast`, `faster`, `fast`, `medium`, `slow`, `slower`, `veryslow` |
| `--ffmpeg PATH` | `ffmpeg` in the PATH | |

`soccerwindow2 --help` lists them too (`Export Options`). The options are given as `--name value` or `--name=value`.
Giving one of them without `--export-video` or `--export-images` is a mistake, except `--export-image-format` and `--export-image-prefix`: without an export, soccerwindow2 starts with the window, and the export dialog starts with them.

### Ranges on the command line

| `RANGE` | |
|---|---|
| `START-END` | from START to END, written as in the dialog (`1234`, `1234.3`, `1234.0`). An end may be empty: `-300` from the first frame, `3000-` to the last one |
| `all` | the whole log |
| `first-half`, `second-half`, `half-N` | a half (`half-3` and on with more than two halves) |
| `extra-N` | a half of the extra time |
| `penalties` | the penalty shootout |
| `game` | the whole game, when the halves have no time |
| `goals` | a range for each goal, from 10 s before it to 5 s after it (the seconds of the playback, as the [presets](#presets)), named `goal1-c1957`, ... |
| `goals:BEFORE:AFTER` | the same, from BEFORE seconds before each goal to AFTER seconds after it |

Several ranges are written each in its own video (`FILE-NAME.ext`, e.g. `goals-goal1-c1957.mp4`) or directory (`DIR/NAME/`), named by the ranges (a `START-END` range by its text), or in one with `--export-join`.
A single range is written to `FILE` itself (`goals.mp4` for a game log with one goal).
A part not in the log is a mistake; `goals` on a log without a goal gives no range, and is a mistake when it is the only range.

### Examples

```bash
# the goals, a video each: goals-goal1-c1957.mp4, goals-goal2-c3466.mp4, ...
soccerwindow2 --export-video goals.mp4 --export-range goals GAME_LOG.rcg.gz

# the first half as one video at twice the speed of the game
soccerwindow2 --export-video first-half.mp4 --export-range first-half --export-speed 2 GAME_LOG.rcg.gz

# images of the cycles 1200 to 1500 in full HD
soccerwindow2 --export-images frames --export-range 1200-1500 --export-size 1920x1080 GAME_LOG.rcg.gz

# the goals, from 5 s before to 3 s after each, in one video, in the paint style of rcssmonitor
soccerwindow2 --export-video goals-joined.mp4 --export-range goals:5:3 --export-join \
  --paint-style rcssmonitor GAME_LOG.rcg.gz

# every game log of a directory
for log in logs/*.rcg.gz; do
  soccerwindow2 --export-video "videos/$(basename "$log" .rcg.gz).mp4" --export-range goals --export-join "$log" || echo "failed: $log"
done
```

The standard error tells, e.g.:

```
ffmpeg version 6.1.1-3ubuntu5; H.264 by libx264
Exporting 604 images of 604 frames in 4 ranges, 1280 x 720 pixels, 10 images/s, to 4 videos (goals-goal1-c1957.mp4, ...)
Exported 61 of 604 images (10%)
...
Exported 604 images; finishing the video
Exported goals-goal1-c1957.mp4
Exported goals-goal2-c3466.mp4
Exported goals-goal3-c3645.mp4
Exported goals-goal4-c5168.mp4
Finished in 1.7 s
```

### Without a display

The command line export shows no window: it runs on Qt's platform `offscreen`, whatever the desktop sets in `QT_QPA_PLATFORM` (Qt's option `-platform` still chooses another).
It needs no display: it runs on a server, over ssh, or in a container.

### Instead of --auto-image-save

The old options `--auto-image-save`, `--image-save-dir`, `--image-name-prefix` and `--image-save-format` are removed (soccerwindow2 tells them as unsupported options and quits).
Their export of the whole game log is made by `--export-images`, without a window:

```bash
# before
soccerwindow2 --auto-image-save --image-save-dir frames --image-name-prefix shot- --image-save-format png GAME_LOG.rcg.gz
# now
soccerwindow2 --export-images frames --export-image-prefix shot- --export-image-format png GAME_LOG.rcg.gz
```

The images are named by the numbers of the frames as before (`shot-00000.png`, ...).
They are 1280 x 720 by default (the old ones were as large as the field view): give `--export-size WxH`, and `--export-scale 1` to lay out a larger image as a field view of its size, as the old ones were.

## Sizes of the files

Some sizes, to plan the disk space (they depend on the game, the settings and the machine):

| What | Size | Time to export |
|---|---|---|
| a PNG of 1280 x 720 | about 90 KB | |
| a PNG of 1920 x 1080 | about 140 KB | |
| a video, 1280 x 720, 10 images/s, CRF 18: a half (5.3 min) | about 38 MB | about 7 s |
| a video, 1280 x 720, 10 images/s, CRF 18: a game (10.7 min) | about 75 MB | about 14 s |

A whole game in PNG images of 1920 x 1080 is about 6,400 images: about 1 GB. A video is much smaller than its images.

## Tips

soccerwindow2 exports the frames of the game as they are. Editing the videos is left to ffmpeg: the recipes below use its filters (ffmpeg 4.2 or later).
They encode again with the settings of soccerwindow2's videos; give `-framerate` the images a second of the export (10 by default).
In them, `goals.mp4` is a video of one goal scene, 15.1 s long (e.g. `--export-range goals` of a game log with one goal).

### Still frames at the start and the end

A video starts with the game moving and ends as the play does. The filter `tpad` holds its first frame and its last frame for some seconds (here 2 and 3):

```bash
ffmpeg -i goals.mp4 \
  -vf "tpad=start_mode=clone:start_duration=2:stop_mode=clone:stop_duration=3" \
  -c:v libx264 -crf 18 -preset medium -tune animation -pix_fmt yuv420p \
  -movflags +faststart goals-still.mp4
```

`-movflags +faststart` lets a browser play the video while it is loading.
A longer range (e.g. `goals:15:8`, from 15 s before each goal) gives a longer lead-in, with the game moving.

### Pauses between the scenes

To have still frames around each scene of several ranges in one video, export a video for each range (without `--export-join`), hold the ends of each, and join them without encoding again:

```bash
soccerwindow2 --export-video goals.mp4 --export-range goals GAME_LOG.rcg.gz
for f in $(ls -v goals-goal*.mp4); do
  ffmpeg -y -i "$f" \
    -vf "tpad=start_mode=clone:start_duration=2:stop_mode=clone:stop_duration=2" \
    -c:v libx264 -crf 18 -preset medium -tune animation -pix_fmt yuv420p "pad-$f"
done
printf "file '%s'\n" $(ls -v pad-goals-goal*.mp4) > list.txt
ffmpeg -f concat -safe 0 -i list.txt -c copy -movflags +faststart goals-all.mp4
```

`ls -v` puts the scenes in the order of their numbers (`goal10` after `goal9`).
The videos joined by `-c copy` must have the same size, rate and encoding, as these do.

### One compression from the images

Each recipe above compresses a compressed video again. At CRF 18 it is hard to see, but the images exported by soccerwindow2 can be made into the video at once instead:

```bash
soccerwindow2 --export-images frames --export-range 1200-1500 GAME_LOG.rcg.gz
ffmpeg -framerate 10 -i frames/image-%05d.png \
  -vf "tpad=start_mode=clone:start_duration=2:stop_mode=clone:stop_duration=3" \
  -c:v libx264 -crf 18 -preset medium -tune animation -pix_fmt yuv420p \
  -movflags +faststart scene.mp4
```

Or export the video without loss first (`--export-crf 0`, a large file), and compress it once at the end.

### Fading from and to black

The filter `fade` fades in at the start and out at the end; the fade out needs the length of the video. With the still frames, in one go (a video of 15.1 s, made 20.1 s long):

```bash
ffmpeg -i goals.mp4 \
  -vf "tpad=start_mode=clone:start_duration=2:stop_mode=clone:stop_duration=3,fade=t=in:st=0:d=1,fade=t=out:st=19.1:d=1" \
  -c:v libx264 -crf 18 -preset medium -tune animation -pix_fmt yuv420p goals-faded.mp4
```

The length of a video, to find where the fade out starts (its length - 1 s):

```bash
ffprobe -v error -show_entries format=duration -of csv=p=0 goals-still.mp4
```

## When something goes wrong

**`Export` is disabled.**
The summary above it tells why in red: no game log, a position not in the log, the same frames named by the cycles twice in one sequence, ffmpeg not found, a video file of another extension. While connected to a server, the tooltip of `Export` tells so: disconnect, and the frames kept can be exported.

**The range has more frames than expected at its end.**
As the end, `1234` is the last frame of the cycle 1234, the frames stopped in it too (e.g. the 50 frames after a goal). Write `1234.0` for the frame before them; `Now`, `]` and the presets write it so.

**The images look cut, or the texts too large or too small.**
Check `Enlarge the texts, logos and lines with the image`: on, the image is laid out as the field view; off, the texts keep their sizes in pixels. A size of another shape than the view shows more or less of the field around it.

**`ffmpeg ... is not found`, or the video cannot be made.**
Install ffmpeg (`sudo apt install ffmpeg`), or choose it with `Browse...` (`--ffmpeg PATH` on the command line). When ffmpeg fails, the message ends with what it said.

**`no H.264: MPEG-4 Part 2 by mpeg4`.**
This ffmpeg has no H.264 encoder: the video is made, larger and of lower quality, and some players do not play it. An ffmpeg with libx264 makes H.264.

**The video does not play.**
A video stopped before its end is removed, as it may not play. `.mp4` plays in most players and browsers; some browsers do not play `.mkv`.

**The debug log of an agent is missing in the images.**
It is drawn only in the frame synchronized with the `Debug Message` window ([What is drawn](#what-is-drawn)).

**A script that used `--auto-image-save` fails.**
Use `--export-images` ([Instead of --auto-image-save](#instead-of---auto-image-save)).
