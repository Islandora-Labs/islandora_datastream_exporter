<?php

/**
 * @file
 * API functionality.
 */

/**
 * Allows for sources of PID lists to be provided to the exporter.
 *
 * @return array
 *   An associative array pairing PID list source names with definitions,
 *   including:
 *   - 'file': A file to load, using an array of parameters as one would pass to
 *     module_load_include(). Not required.
 *   - 'validation_callback': A function to call to validate the drush script's
 *     query parameter. Not required.
 *   - 'pid_source_callback': A function to call to get a list of PIDs.
 *     Required.
 *   - 'description': Some descriptive information for the query type. Not 
 *     required.
 */
function hook_islandora_datastream_exporter_pid_source() {
  return array(
    'my_cool_exporter' => array(
      'file' => array('inc', 'my_cool_exporter', 'includes/exporter'),
      'validation_callback' => 'my_cool_exporter_validation_callback',
      'pid_source_callback' => 'my_cool_exporter_pid_source_callback',
      'description' => t('Some cool exporter'),
    ),
  );
}

/**
 * Example validation callback.
 *
 * @param string $query
 *   The drush script query parameter.
 *
 * @return bool
 *   FALSE if we should not continue; TRUE otherwise. Prefer to return the
 *   result of drush_set_error() if issues occur so that messaging is provided.
 */
function my_cool_exporter_validation_callback($query) {
  if ($query !== 'totally valid query') {
    return drush_set_error('Nope', 'This is not even valid what are you doing');
  }
  return TRUE;
}

/**
 * Example PID source callback.
 *
 * @param string $query
 *   The drush script's query parameter.
 * @param int $max
 *   The maximum number of PIDs to return.
 * @param array|DrushBatchContext $context
 *   The batch context, passed in by reference, so that messaging and sandboxing
 *   can be provided. It is also expected that this callback will set the
 *   'finished' parameter.
 *
 * @return string[]
 *   An array containing PIDs as strings. If an empty array is returned, batch
 *   execution will stop.
 */
function my_cool_exporter_pid_source_callback($query, $max, &$context) {
  $sandbox =& $context['sandbox'];
  if (empty($sandbox) {
    $sandbox['offset'] = 0;
  }
  $pids = do_something_cool_with_query($query, $max, $sandbox['offset']);
  $sandbox['offset'] += count($pids);
  return $pids;
}
