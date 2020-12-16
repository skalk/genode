content: no_decorator_theme.tar wm.config

no_decorator_theme.tar:
	tar -c -f $@ -C $(REP_DIR)/recipes/raw/test-mobile_shell theme
	tar -r -f $@ -C $(REP_DIR)/src/app/themed_decorator theme/closer.png theme/default.png theme/maximizer.png theme/font.tff

wm.config:
	cp $(REP_DIR)/recipes/raw/test-mobile_shell/$@ $@

