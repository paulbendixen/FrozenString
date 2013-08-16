require 'terminfo'
require 'parallel'
require 'securerandom'

notification :terminal_notifier

def separator
  "\e[1;33m" + '~' * (TermInfo.screen_size[1]-1) + "\e[0m"
end

def compile file
  out = "#{File.basename file, ".*"}#{SecureRandom.uuid}.out"
  Parallel.map(
    [
      [
        "g++-4.8",
        "-std=c++11 -Wall -Wextra -pedantic #{file} -o #{out}",
        "./#{out}; if [ -f '#{out}' ]; then rm #{out}; fi",
      ],
      [
        "clang++",
        "-std=c++11 -stdlib=libc++ -Wall -Wextra #{file} -o #{out}",
        "./#{out}; if [ -f '#{out}' ]; then rm #{out}; fi",
      ]
    ],
    in_threads: 2
  ) do |compiler, options, exe|
    result = `#{compiler} #{options} && #{exe}`
    puts "compiling #{File.basename file} with #{compiler}...#{result}"
    $?.success?
  end.inject{|a,i| a && i}
end

def which cmd
  dir = ENV['PATH'].split(':').find {|p| File.executable? File.join(p, cmd)}
  File.join(dir, cmd) unless dir.nil?
end

def notify failed
  msg = "'#{failed} test#{failed>1 ? "s" : ""} failed.\n#{Time.now.to_s}'"
  case
  when which('terminal-notifier')
    `terminal-notifier -message #{msg}`
  when which('notify-send')
    `notify-send #{msg}`
  when which('tmux')
    `tmux display-message #{msg}` if `tmux list-clients 1>/dev/null 2>&1` && $?.success?
  end
end

guard :shell do
  watch %r{^.+\.(?:hpp|cpp)$} do
    puts separator
    puts Time.now.to_s

    failed, total = 0, 0
    Parallel.map(Dir.glob("tests/**/*.cpp"), in_threads: 8) do |f|
      success = compile f
      failed += 1 unless success
      total += 1
    end

    notify failed if failed > 0
    (failed==0 ? "Success!" : "#{failed} failed (#{failed}/#{total})")
  end
end
