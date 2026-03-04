filename = 'out_192_168_10_5_25001/SensorData.bin';
fid = fopen(filename,'rb');
if fid < 0, error('Cannot open file'); end

% Read header (32 bytes)
magic = char(fread(fid,8,'char')');
version = fread(fid,1,'uint32');
numValues = fread(fid,1,'uint32');
sizeof_double = fread(fid,1,'uint8');
endian_flag = fread(fid,1,'uint8'); % 1=little endian
fread(fid,6,'uint8'); % reserved
start_epoch = fread(fid,1,'uint64');

fprintf('magic=%s version=%d numValues=%d sizeof_double=%d endian=%d\n',magic,version,numValues,sizeof_double,endian_flag);

% Reader helper: compute CRC in MATLAB using java CRC32
crc = java.util.zip.CRC32();

rec_idx = 0;
while ~feof(fid)
    % read seq (u64 little-endian)
    seq = fread(fid,1,'uint64=>uint64','l'); % little-endian
    if isempty(seq), break; end

    ts_us = fread(fid,1,'uint64=>uint64','l');
    % read values (little-endian doubles)
    vals = fread(fid, numValues, 'double=>double','l') ;
    if numel(vals) < numValues
        warning('Unexpected EOF in values');
        break;
    end

    % read CRC (u32 LE)
    crc_read = fread(fid,1,'uint32=>uint32','l');

    % compute CRC over payload: seq(8) + ts(8) + values(numValues*8)
    % construct uint8 array identical to C++ payload (little-endian)
    payload = uint8([]);
    payload = [payload ; typecast(uint64(seq),'uint8')(:)]; % this returns platform order
    payload = [payload ; typecast(uint64(ts_us),'uint8')(:)];
    for k=1:numValues
        payload = [payload ; typecast(double(vals(k)),'uint8')(:)];
    end

    % Java CRC expects big-endian byte order in the byte array provided;
    % but we used the same byte sequence the C++ code used (little-endian order).
    crc.reset();
    crc.update(payload);
    crc_calc = uint32(crc.getValue());

    if crc_calc ~= uint32(crc_read)
        fprintf('CRC mismatch at record %d: seq=%d, crc_calc=0x%08X, crc_read=0x%08X\n', rec_idx, seq, crc_calc, crc_read);
    end

    % use vals (row vector) or store to matrix:
    % e.g., data(rec_idx+1,:) = vals';
    rec_idx = rec_idx + 1;
end

fclose(fid);
fprintf('Read %d records\n', rec_idx);