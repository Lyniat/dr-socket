class IOSWizard < Wizard
  def before_create_payload
    puts "* INFO - before_create_payload"
    puts "#{app_path}"

    # stage extension
    sh "cp ./dr-socket/native/ios-device/Info.plist ./dr-socket/native/ios-device/socket.framework/Info.plist"
    sh "mkdir -p \"#{app_path}/Frameworks/socket.framework/\""
    sh "cp -r \"#{root_folder}/native/ios-device/socket.framework/\" \"#{app_path}/Frameworks/socket.framework/\""

    # sign
    sh <<-S
    CODESIGN_ALLOCATE=#{codesign_allocate_path} #{codesign_path} \\
                                                -f -s \"#{certificate_name}\" \\
                                                \"#{app_path}/Frameworks/socket.framework/socket\"
    S
   end
end