<?php
/**
 * @file
 * Drush command/hook implementation for exporting datastreams.
 */

/**
 * Implements hook_drush_command().
 */
function islandora_datastream_exporter_drush_command() {
  $commands = array();

  $commands['islandora_datastream_export'] = array(
    'description' => dt('Exports a specified datastream from all objects given a fielded Solr query.'),
    'drupal dependencies' => array(
      'islandora',
    ),
    'options' => array(
      'query_type' => array(
        'description' => dt('The type of query to run. Check the output of "drush islandora_datastream_export_types" for a list. Defaults to "islandora_datastream_exporter_solr_query".'),
        'required' => FALSE,
      ),
      'query' => array(
        'description' => dt('The query to be ran.'),
        'required' => TRUE,
      ),
      'dsid' => array(
        'description' => dt('The datastream id of to be exported datastream.'),
        'required' => TRUE,
      ),
      'export_target' => array(
        'description' => dt('The directory to export the datastreams to.'),
        'required' => TRUE,
      ),
    ),
    'examples' => array(
      'drush -u 1 islandora_datastream_export --export_target=/tmp --query=PID:\"islandora:9\" --dsid=DC' => dt('Exporting datastream from object via default Solr query.'),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  $commands['islandora_datastream_export_types'] = array(
    'description' => dt('Lists the types of exporters available.'),
    'examples' => array('drush islandora_datastream_export_types'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  return $commands;
}

/**
 * Batch process validation handler.
 */
function drush_islandora_datastream_exporter_islandora_datastream_export_validate() {
  $types = module_invoke_all('islandora_datastream_exporter_pid_source');
  $type = drush_get_option('query_type', 'islandora_datastream_exporter_solr_query');
  if (!isset($types[$type])) {
    return drush_set_error('Invalid type', dt('The specified type, !type, is not valid.', array('!type' => $type)));
  }
  $directory = drush_get_option('export_target');
  if (!is_writable($directory)) {
    return drush_set_error('This is not is a directory', dt('The specified target directory, !dir, is not valid.', array('!dir' => $directory)));
  }
  $query = drush_get_option('query');
  if (isset($types[$type]['validation_callback'])) {
    if (isset($types[$type]['file'])) {
      call_user_func_array('module_load_include', $types[$type]['file']);
    }
    return call_user_func_array($types[$type]['validation_callback'], array($query));
  }
}

/**
 * Command callback to export datastreams.
 */
function drush_islandora_datastream_exporter_islandora_datastream_export() {
  batch_set(islandora_datastream_export_create_batch());
  drush_backend_batch_process();
}

/**
 * Constructs a batch used to update things via Drush.
 */
function islandora_datastream_export_create_batch() {
  $types = module_invoke_all('islandora_datastream_exporter_pid_source');
  $type = drush_get_option('query_type', 'islandora_datastream_exporter_solr_query');
  return array(
    'operations' => array(
      array('islandora_datastream_export_batch_operation',
        array(
          $types[$type],
          drush_get_option('query'),
          drush_get_option('dsid'),
          drush_get_option('export_target'),
        ),
      ),
    ),
    'title' => t('Exporting datastreams...'),
    'init_message' => t('Preparing to export datastream.'),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_datastream_exporter') . '/islandora_datastream_exporter.drush.inc',
  );
}

/**
 * Constructs and performs the datastream export operation.
 *
 * @param array $type
 *   A type definition array for the PID source.
 * @param string $query
 *   The Solr query to be used for searching.
 * @param string $dsid
 *   The datastream ID of the object to be exported.
 * @param string $target
 *   The export_target directory to export the files to.
 * @param array $context
 *   The context of the Drupal batch.
 */
function islandora_datastream_export_batch_operation($type, $query, $dsid, $target, &$context) {
  drupal_static_reset('islandora_get_tuque_connection');
  $export_objects = 10;
  if (isset($type['file'])) {
    call_user_func_array('module_load_include', $type['file']);
  }
  module_load_include('inc', 'islandora', 'includes/mimetype.utils');
  module_load_include('inc', 'islandora', 'includes/utilities');
  $results = call_user_func_array($type['pid_source_callback'], array(
    $query,
    $export_objects,
    &$context,
  ));
  foreach ($results as $pid) {
    $object = islandora_object_load($pid);
    if (isset($object[$dsid])) {
      $extension = islandora_get_extension_for_mimetype($object[$dsid]->mimeType);
      $file_pid = islandora_escape_pid_for_function($object->id);
      $target_directory = "$target/{$file_pid}_$dsid.$extension";
      $object[$dsid]->getContent($target_directory);
      $success = TRUE;
    }
    else {
      $success = FALSE;
    }
    if ($success) {
      drush_log(dt("Datastream exported succeeded for @pid.", array('@pid' => $object->id)), 'success');
    }
    else {
      drush_log(dt("Datastream export failed for @pid. The object does not contain the @dsid datastream.", array(
        '@pid' => $object->id,
        '@dsid' => $dsid,
      )), 'error');
    }
  }
}

/**
 * Lists the types of exporters.
 */
function drush_islandora_datastream_exporter_islandora_datastream_export_types() {
  foreach (module_invoke_all('islandora_datastream_exporter_pid_source') as $type => $source) {
    if (isset($source['description'])) {
      drush_print("$type: {$source['description']}");
    }
    else {
      drush_print($type);
    }
  }
}
