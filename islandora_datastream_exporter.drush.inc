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
    'description' => dt('Exports a specified datastream from all objects given a Solr filter query.'),
    'drupal dependencies' => array(
      'islandora',
      'islandora_solr',
    ),
    'options' => array(
      'field' => array(
        'description' => t('The Solr field being queried.'),
        'required' => TRUE,
      ),
      'value' => array(
        'description' => t('The value to be searched for within the Solr field.'),
        'required' => TRUE,
      ),
      'dsid' => array(
        'description' => t('The datastream id of to be exported datastream.'),
        'required' => TRUE,
      ),
      'target' => array(
        'description' => t('The directory to export the datastreams to.'),
        'required' => TRUE,
      ),
    ),
    'examples' => array(
      'drush -u 1 islandora_datastream_export --target=/tmp --field=* --value=* --dsid=DC' => dt('Exporting datastream from object.'),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  return $commands;
}

/**
 * Batch process validation handler.
 */
function drush_islandora_datastream_exporter_islandora_datastream_export_validate() {
  $directory = drush_get_option('target');
  if (!is_writable($directory)) {
    return drush_set_error('This is not is a directory', dt('The specified target directory, !dir, is not valid.', array('!dir' => $directory)));
  }
  $query = $query = format_string('!field:!value', array(
    '!field' => drush_get_option('field'),
    '!value' => drush_get_option('value'),
  ));
  $qp = new IslandoraSolrQueryProcessor();
  $qp->buildQuery($query);
  $qp->executeQuery(FALSE);
  if (empty($qp->islandoraSolrResult)) {
    return drush_set_error('Invalid Solr query', dt('The specified query !query is not valid. Please ensure you have a correct and escaped query.', array('!query' => $query)));
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
  return array(
    'operations' => array(
      array('islandora_datastream_export_batch_operation',
        array(
          drush_get_option('field'),
          drush_get_option('value'),
          drush_get_option('dsid'),
          drush_get_option('target'),
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
 * @param string $field
 *   The Solr field to be queried.
 * @param string $value
 *   The value to queried for.
 * @param string $dsid
 *   The datastream ID of the object to be exported.
 * @param string $target
 *   The target directory to export the files to.
 * @param array $context
 *   The context of the Drupal batch.
 */
function islandora_datastream_export_batch_operation($field, $value, $dsid, $target, &$context) {
  $sandbox = &$context['sandbox'];
  $export_objects = 10;
  $query = format_string('!field:!value', array(
    '!field' => $field,
    '!value' => $value,
  ));
  $qp = new IslandoraSolrQueryProcessor();
  $qp->buildQuery($query);
  if (!isset($sandbox['offset'])) {
    $qp->executeQuery(FALSE);
    $sandbox['total'] = $qp->islandoraSolrResult['response']['numFound'];
    $sandbox['offset'] = 0;
  }
  $context['message'] = t('Processing results @start to @end.', array(
    '@start' => $sandbox['offset'],
    '@end' => min($sandbox['offset'] + $export_objects, $sandbox['total']),
  ));
  $qp->solrLimit = $export_objects;
  $qp->solrStart = $sandbox['offset'];
  $qp->executeQuery();
  $results = $qp->islandoraSolrResult['response']['objects'];
  foreach ($results as $solr_doc) {
    module_load_include('inc', 'islandora', 'includes/mimetype.utils');
    $object = islandora_object_load($solr_doc['PID']);
    if (isset($object[$dsid])) {
      $extension = islandora_get_extension_for_mimetype($object[$dsid]->mimeType);
      $file_pid = str_replace(':', '_', $object->id);
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
  $sandbox['offset'] += $export_objects;
  $context['finished'] = $sandbox['offset'] / $sandbox['total'];
}
