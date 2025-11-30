
Dir.glob(File.join(__dir__, "test_*.rb")).each(&Kernel.method(:require))
require "minitest/autorun"
