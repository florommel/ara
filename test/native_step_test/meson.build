test('native-steps',
    py3_inst,
    args: files('native_steps.py'),
    env: [python_path],
    suite: 'steps'
)

test('bb-split',
    py3_inst,
    args: [files('run_test_step.py'), 'BBSplitTest', gpslogger_application],
    env: [python_path],
    suite: 'steps'
)

test('cfg-optimize',
    py3_inst,
    args: [files('run_test_step.py'), 'CFGOptimizeTest', freertos_optimization],
    env: [python_path],
    suite: 'steps'
)

test('comp-insert',
    py3_inst,
    args: [files('run_test_step.py'), 'CompInsertTest', gpslogger_application],
    env: [python_path],
    suite: 'steps'
)

test('fn-single-exit',
    py3_inst,
    args: [files('run_test_step.py'), 'FnSingleExitTest', freertos_multi_exit],
    env: [python_path],
    suite: 'steps'
)

test('llvm-map',
    py3_inst,
    args: [files('run_test_step.py'), 'LLVMMapTest', freertos_icfg],
    env: [python_path],
    suite: 'steps'
)
